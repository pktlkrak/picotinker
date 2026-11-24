use std::collections::HashMap;

use anyhow::{Result, anyhow, bail};
use elf::{
    ElfBytes, abi::{PT_LOAD, R_ARM_GLOB_DAT, R_ARM_JUMP_SLOT, R_ARM_RELATIVE}, endian::AnyEndian,
};

use crate::utils::{get_u32, set_u32};

#[derive(Debug)]
pub struct ApplicationDescription {
    pub rom_image: Vec<u8>,
    pub ram_image: Vec<u8>,
    pub metadata_offset: usize,
    pub dynamic_ref_map: HashMap<usize, String>,
    pub own_relocs_map: HashMap<usize, usize>,

    pub c_app_definition: CAppDefinition,
    pub name: String,
}

const START_OF_DYNAMIC_RAM: usize = 0x20060000usize;

const ALLOWED_UNDEF_SYMBOLS: [&'static str; 4] = [
    "__libc_init_array",
    "__register_frame_info",
    "atexit",
    "__deregister_frame_info"
];

#[derive(Debug)]
#[repr(C)]
pub struct CAppDefinition {
    pub name: u32,
    pub icon: u32,
    pub entry_point: u32,
    pub userdata_ptr_loc: u32,
    pub ram_base: u32,
    pub userdata_base: u32,

    pub ram_size_def: u32,
    pub ram_size_full: u32,
    pub userdata_size: u32,
    pub next: u32,
}

impl ApplicationDescription {
    pub fn reserialize_metadata_structure(&mut self) {
        let mut cursor = self.metadata_offset;
        macro_rules! write_next {
            ($x: expr) => {
                set_u32(&mut self.rom_image, cursor, $x);
                cursor += 4;
            };
        }

        write_next!(self.c_app_definition.name);
        write_next!(self.c_app_definition.icon);
        write_next!(self.c_app_definition.entry_point);
        write_next!(self.c_app_definition.userdata_ptr_loc);
        write_next!(self.c_app_definition.ram_base);
        write_next!(self.c_app_definition.userdata_base);
        write_next!(self.c_app_definition.ram_size_def);
        write_next!(self.c_app_definition.ram_size_full);
        write_next!(self.c_app_definition.userdata_size);
        write_next!(self.c_app_definition.next);

        let _ = cursor;
    }

    pub fn apply_ref_map(&mut self, system_ref_map: &HashMap<String, usize>) {
        for (offset, value) in &self.dynamic_ref_map {
            if let Some(sys_val) = system_ref_map.get(value) {
                if *offset > START_OF_DYNAMIC_RAM {
                    set_u32(
                        &mut self.ram_image,
                        *offset - START_OF_DYNAMIC_RAM,
                        *sys_val as u32,
                    );
                } else {
                    set_u32(&mut self.rom_image, *offset, *sys_val as u32);
                }
            } else if !ALLOWED_UNDEF_SYMBOLS.iter().any(|e| e == value) {
                println!(
                    "Warning: Symbol {value} requested by app {} hasn't been found!",
                    self.name
                );
            }
        }
    }

    pub fn update_metadata_structure(&mut self, base: u32, ram_base: u32) {
        self.c_app_definition.name += base;
        self.c_app_definition.icon += base;
        self.c_app_definition.entry_point += base;
        self.c_app_definition.userdata_ptr_loc += base;
        self.c_app_definition.ram_base = ram_base;
        self.c_app_definition.userdata_base += base;

        for (rel, zero_relative_offset) in &self.own_relocs_map {
            let mut new_val = if *zero_relative_offset == usize::MAX {
                if *rel >= START_OF_DYNAMIC_RAM {
                    get_u32(&mut self.ram_image, *rel - START_OF_DYNAMIC_RAM)
                } else {
                    get_u32(&mut self.rom_image, *rel)
                }
            } else {
                *zero_relative_offset as u32
            };
            if new_val < START_OF_DYNAMIC_RAM as u32 {
                new_val += base;
            }
            if *rel >= START_OF_DYNAMIC_RAM {
                set_u32(&mut self.ram_image, *rel - START_OF_DYNAMIC_RAM, new_val);
            } else {
                set_u32(&mut self.rom_image, *rel, new_val);
            }
        }
    }

    pub fn load_from_elf(elf_file_path: &str) -> Result<Self> {
        let file_contents = std::fs::read(elf_file_path)?;
        let file = ElfBytes::<AnyEndian>::minimal_parse(&file_contents)?;

        let mut dynamic_ref_map = HashMap::new();
        let mut own_relocs_map = HashMap::new();
        let mut ram_size = 0usize;
        let mut rom_image = None;
        let mut ram_image = None;
        let mut metadata_offset = 0;

        let rel_dyn = file
            .section_header_by_name(".rel.dyn")?
            .ok_or(anyhow!("Dynamic relocations section required!"))?;
        let rel_dyn_contents = file.section_data_as_rels(&rel_dyn)?;
        let symtab_for_rel_dyn = file
            .dynamic_symbol_table()?
            .ok_or(anyhow!("Symbol table required!"))?;

        let mut entry_point = 0xFFFFFFFFu32;
        // Find bindings to the OS
        for entry in rel_dyn_contents {
            let dynamic_symbol = symtab_for_rel_dyn.0.get(entry.r_sym as usize).unwrap();
            let dynamic_symbol_name = symtab_for_rel_dyn
                .1
                .get(dynamic_symbol.st_name as usize)
                .unwrap();
            if entry.r_type == R_ARM_JUMP_SLOT && dynamic_symbol.is_undefined() {
                // Function from the system.
                dynamic_ref_map.insert(entry.r_offset as usize, dynamic_symbol_name.to_string());
            } else if entry.r_type == R_ARM_JUMP_SLOT || (entry.r_type == R_ARM_GLOB_DAT && !dynamic_symbol.is_undefined()){
                // But defined.
                own_relocs_map.insert(entry.r_offset as usize, dynamic_symbol.st_value as usize);
            } else if entry.r_type == R_ARM_RELATIVE {
                own_relocs_map.insert(entry.r_offset as usize, usize::MAX);
            }

            if dynamic_symbol_name == "main" {
                // in APPLICATION there's actually a reloc probably :(
                // i don't like this solution
                entry_point = dynamic_symbol.st_value as u32;
            }
        }

        if entry_point == 0xFFFFFFFFu32 {
            bail!("No main() defined in '{elf_file_path}'!");
        }

        // And the application definition:
        for entry in symtab_for_rel_dyn.0 {
            if symtab_for_rel_dyn.1.get(entry.st_name as usize).unwrap() == "APPLICATION" {
                metadata_offset = entry.st_value as usize;
            }
        }

        // Decode segments:
        for segment in file.segments().expect("Duh") {
            if segment.p_type == PT_LOAD && segment.p_filesz > 0 {
                match segment.p_vaddr as usize {
                    0 => {
                        // ROM
                        rom_image = Some(
                            &file_contents[segment.p_offset as usize
                                ..(segment.p_offset + segment.p_filesz) as usize],
                        );
                    }
                    START_OF_DYNAMIC_RAM => {
                        ram_image = Some(
                            &file_contents[segment.p_offset as usize
                                ..(segment.p_offset + segment.p_filesz) as usize],
                        );
                        ram_size = segment.p_memsz as usize;
                    }
                    0xF000_0000 => { /* Discard */ }

                    other => bail!(
                        "Unknown ELF format definining loadable segment at {other:x} in app '{elf_file_path}' (did you move your dynamic RAM?)!"
                    ),
                }
            }
        }

        if ram_image.is_none() || rom_image.is_none() {
            bail!("RAM or ROM not defined in '{elf_file_path}'!");
        }

        if metadata_offset == 0 {
            bail!("APPLICATION metadata is not defined in '{elf_file_path}'!");
        }

        if metadata_offset > START_OF_DYNAMIC_RAM {
            bail!("APPLICATION being defined in RAM is not allowed ('{elf_file_path}')!");
        }

        let mut meta_cursor = metadata_offset;
        macro_rules! read_u32_metadata {
            () => {{
                let out = get_u32(rom_image.unwrap(), meta_cursor);
                meta_cursor += 4;
                out
            }};
        }
        let mut c_app_definition = CAppDefinition {
            name: read_u32_metadata!(),
            icon: read_u32_metadata!(),
            entry_point: read_u32_metadata!(),
            userdata_ptr_loc: read_u32_metadata!(),
            ram_base: read_u32_metadata!(),
            userdata_base: read_u32_metadata!(),

            ram_size_def: read_u32_metadata!(),
            ram_size_full: read_u32_metadata!(),
            userdata_size: read_u32_metadata!(),
            next: read_u32_metadata!(),
        };

        c_app_definition.entry_point = entry_point;

        c_app_definition.ram_size_def = ram_image.unwrap().len() as u32;
        c_app_definition.ram_size_full = ram_size as u32;

        let _ = meta_cursor;

        let substr = &rom_image.unwrap()[c_app_definition.name as usize..];
        let name =
            String::from_utf8(substr[..substr.iter().position(|e| *e == 0).unwrap()].to_vec())
                .unwrap();

        Ok(ApplicationDescription {
            ram_image: ram_image.unwrap().to_vec(),
            rom_image: rom_image.unwrap().to_vec(),
            dynamic_ref_map,
            metadata_offset,
            c_app_definition,
            own_relocs_map,
            name,
        })
    }
}
