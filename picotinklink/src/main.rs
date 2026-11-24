mod subapp;
mod system;
mod uf2;
mod utils;

use std::ops::Range;

use clap::Parser;

use crate::{subapp::ApplicationDescription, uf2::serialize_bin_to_uf2, utils::set_u32};

const SYSTEM_FLASH_RANGE: Range<usize> = 0x1000_0000usize..0x1100_0000usize;

struct LinkedApplicationDescription {
    base_rom_address: usize,
    base_ram_address: usize,
    app_description: ApplicationDescription,
}

#[derive(Parser)]
#[command(
    name = "picotinklink",
    version = "0.1.0",
    author = "asivery",
    about = "Second-stage linker for Picotinker"
)]
struct Cli {
    /// The base system / environment / HIL image
    base_system: String,
    /// Total space the Pico has available
    #[arg(short, long, default_value = "2097152")] // 2 MiB
    total_space: usize,
    /// Space reserved for applications' userdata (settings / progress in games)
    #[arg(short, long, default_value = "131072")] // 128 KiB
    userdata_space: usize,
    /// Whether or not to keep userdata (MUST be set to false if first time writing firmware or userdata_space changed!)
    #[arg(long, action = clap::ArgAction::SetTrue)]
    keep_userdata: bool,
    /// The apps to install
    applications: Vec<String>,
    /// The output, final UF2 file
    #[arg(short)]
    output: String,
}

fn main() {
    let cli = Cli::parse();

    // Prepare the FLASH image to work in
    let mut flash = vec![0u8; cli.total_space];
    let sys_info = crate::system::load_base_system(&mut flash, &cli.base_system).unwrap();

    let mut app_descriptions = Vec::with_capacity(cli.applications.len());
    for app_name in cli.applications {
        app_descriptions.push(ApplicationDescription::load_from_elf(&app_name).unwrap());
    }

    // Allocate the applications and RAM images
    let mut current_cursor = sys_info.used_range.end as usize - SYSTEM_FLASH_RANGE.start;
    macro_rules! align_cursor {
        () => {
            if (current_cursor & 0xFF) > 0 {
                current_cursor = (current_cursor & !0xFFusize) + 0x100;
            }
        };
    }

    let mut linked_applications: Vec<LinkedApplicationDescription> =
        Vec::with_capacity(app_descriptions.len());

    for mut app_description in app_descriptions {
        align_cursor!();
        let base_rom_address = current_cursor;

        // Update the definitions in the app linked list structure
        flash[current_cursor..current_cursor + app_description.rom_image.len()]
            .copy_from_slice(&app_description.rom_image);

        current_cursor += app_description.rom_image.len();
        align_cursor!();
        let base_ram_address = current_cursor;
        current_cursor += app_description.ram_image.len();

        // Bind the actual offsets in the struct, so that the data is valid.
        app_description.update_metadata_structure(
            (base_rom_address + SYSTEM_FLASH_RANGE.start) as u32,
            (base_ram_address + SYSTEM_FLASH_RANGE.start) as u32,
        );

        // Bind the linked list:
        if let Some(previous_application) = linked_applications.last_mut() {
            previous_application.app_description.c_app_definition.next =
                (SYSTEM_FLASH_RANGE.start + base_rom_address + app_description.metadata_offset)
                    as u32;
        }

        // Resolve symbols
        app_description.apply_ref_map(&sys_info.system_exports_map);

        println!("Info: Linked app '{}'", app_description.name);
        linked_applications.push(LinkedApplicationDescription {
            base_rom_address,
            base_ram_address,
            app_description,
        });
    }

    // Bind the first application to the linked list's head in the core 'OS':
    if let Some(root_app) = linked_applications.first() {
        set_u32(
            &mut flash,
            sys_info.root_applications_list_ptr_offset as usize,
            (root_app.app_description.metadata_offset
                + root_app.base_rom_address
                + SYSTEM_FLASH_RANGE.start) as u32,
        );
    }

    if let Some(last_application) = linked_applications.last_mut() {
        last_application.app_description.c_app_definition.next = 0u32;
    }

    for x in &mut linked_applications {
        x.app_description.reserialize_metadata_structure();
        flash[x.base_rom_address..x.base_rom_address + x.app_description.rom_image.len()]
            .copy_from_slice(&x.app_description.rom_image);
        flash[x.base_ram_address..x.base_ram_address + x.app_description.ram_image.len()]
            .copy_from_slice(&x.app_description.ram_image);
    }

    if current_cursor & 255 > 0 {
        current_cursor += 256 - (current_cursor & 255);
    }

    if cli.output.to_lowercase().ends_with(".uf2") {
        let uf2_file = serialize_bin_to_uf2(
            &flash[..current_cursor],
            SYSTEM_FLASH_RANGE.start as u32,
            0,
            0,
        )
        .unwrap();
        std::fs::write(&cli.output, &uf2_file).unwrap();
    } else {
        std::fs::write(&cli.output, &flash[..current_cursor]).unwrap();
    }
}
