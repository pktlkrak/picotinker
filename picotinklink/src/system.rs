use std::{collections::HashMap, ops::Range};

use anyhow::{Result, anyhow, bail};
use elf::{ElfBytes, abi::PT_LOAD, endian::AnyEndian};

const SYSTEM_FLASH_RANGE: Range<u64> = 0x1000_0000u64..0x1100_0000u64;

pub struct BaseSystemInfo {
    pub used_range: Range<u64>,
    pub root_applications_list_ptr_offset: usize,
    pub system_exports_map: HashMap<String, usize>,
}

const APPLICATIONS_LINKED_LIST_ROOT_SIGNATURE: [u8; 4] = [0x11, 0x4A, 0xFE, 0xCA];

pub fn load_base_system(flash: &mut [u8], base_system_file: &str) -> Result<BaseSystemInfo> {
    let file_contents = std::fs::read(base_system_file)?;
    let file = ElfBytes::<AnyEndian>::minimal_parse(&file_contents)?;

    let mut base_system_range = Range {
        start: u64::MAX,
        end: 0,
    };
    let mut root_applications_list_ptr_offset = 0;
    let mut resolved_root_applications_list_ptr_offset = false;
    let mut system_exports_map = HashMap::new();

    let symtab = file
        .symbol_table()
        .expect("Should have a symbol table")
        .ok_or(anyhow!("Does not have a symbol table"))?;
    for symbol in symtab.0 {
        let name = symtab.1.get(symbol.st_name as usize)?;
        if name == "APPLICATIONS_LINKED_LIST_ROOT" {
            root_applications_list_ptr_offset = symbol.st_value as usize;
        }
        system_exports_map.insert(String::from(name), symbol.st_value as usize); // System symbols aren't rebound.
    }

    // Merge segments into main flash image:
    for segment in file.segments().expect("Duh") {
        if segment.p_type == PT_LOAD && segment.p_filesz > 0 {
            if !SYSTEM_FLASH_RANGE.contains(&segment.p_paddr) {
                bail!(
                    "Segment which is required to be loaded located outside of SYSTEM FLASH in SYSTEM FLASH IMAGE. ({segment:?})"
                );
            }

            let offset = (segment.p_paddr - SYSTEM_FLASH_RANGE.start) as usize;
            let in_file_offset = segment.p_offset as usize;
            let size = segment.p_filesz as usize;
            flash[offset..offset + size]
                .copy_from_slice(&file_contents[in_file_offset..in_file_offset + size]);

            base_system_range.start = base_system_range.start.min(segment.p_paddr);
            base_system_range.end = base_system_range
                .end
                .max(segment.p_filesz + segment.p_paddr);

            // Resolve the relative pointer to the root of the applications' list
            if !resolved_root_applications_list_ptr_offset
                && root_applications_list_ptr_offset > segment.p_vaddr as usize
                && root_applications_list_ptr_offset < (segment.p_vaddr + segment.p_memsz) as usize
            {
                root_applications_list_ptr_offset =
                    root_applications_list_ptr_offset - segment.p_vaddr as usize + offset;
                resolved_root_applications_list_ptr_offset = true;
            }
        }
    }

    if !resolved_root_applications_list_ptr_offset {
        bail!("Root applications list not found in SYSTEM FLASH IMAGE!");
    }

    let root_list_sig = &flash[root_applications_list_ptr_offset as usize
        ..root_applications_list_ptr_offset as usize + 4usize];
    if root_list_sig != APPLICATIONS_LINKED_LIST_ROOT_SIGNATURE {
        bail!(
            "Root applications list signature mismatch ({root_list_sig:?}@{root_applications_list_ptr_offset:x})!"
        );
    }

    Ok(BaseSystemInfo {
        used_range: base_system_range,
        root_applications_list_ptr_offset,
        system_exports_map,
    })
}
