use anyhow::{Result, bail};

use crate::utils::set_u32;
struct UF2Block<'a> {
    magic_start0: u32,
    magic_start1: u32,
    flags: u32,
    target_addr: u32,
    payload_size: u32,
    block_no: u32,
    num_blocks: u32,
    family_id: u32,
    data: &'a [u8],
    magic_end: u32,
}

const MAGIC_START_0: u32 = 0x0A324655u32;
const MAGIC_START_1: u32 = 0x9E5D5157u32;
const MAGIC_END: u32 = 0x0AB16F30u32;
const PI_PICO2_FAMILY_ID: u32 = 0xe48bff59u32;

impl UF2Block<'_> {
    pub fn serialize_to(&self, out: &mut [u8]) {
        let mut cursor = 0;
        macro_rules! write_next {
            ($x: expr) => {
                set_u32(out, cursor, $x);
                cursor += 4;
            };
        }

        write_next!(self.magic_start0);
        write_next!(self.magic_start1);
        write_next!(self.flags);
        write_next!(self.target_addr);
        write_next!(self.payload_size);
        write_next!(self.block_no);
        write_next!(self.num_blocks);
        write_next!(self.family_id);
        out[cursor..cursor + self.data.len()].copy_from_slice(&self.data);
        out[cursor + self.data.len()..cursor + 476].fill(0);
        cursor += 476;
        write_next!(self.magic_end);

        let _ = cursor;
    }
}

pub fn serialize_bin_to_uf2(
    binary_data: &[u8],
    base_target: u32,
    preceding_blocks: u32,
    additional_blocks: u32,
) -> Result<Vec<u8>> {
    if binary_data.len() % 256 != 0 {
        bail!("Data not 256-byte aligned!");
    }

    // Every 256-byte chunk gets extended to 512 bytes
    let mut final_data = Vec::with_capacity(2 * binary_data.len());
    final_data.resize(2 * binary_data.len(), 0);

    for block_no in 0..binary_data.len() as u32 / 256 {
        let chunk_offset = 256 * block_no as usize;
        let chunk = &binary_data[chunk_offset..chunk_offset + 256];
        let block = UF2Block {
            magic_start0: MAGIC_START_0,
            magic_start1: MAGIC_START_1,
            magic_end: MAGIC_END,

            block_no: block_no + preceding_blocks,
            data: chunk,
            flags: 0x2000,
            target_addr: block_no * 256 + base_target,
            family_id: PI_PICO2_FAMILY_ID,
            payload_size: 256,
            num_blocks: binary_data.len() as u32 / 256 + additional_blocks,
        };
        block.serialize_to(&mut final_data[chunk_offset * 2..chunk_offset * 2 + 512]);
    }

    Ok(final_data)
}
