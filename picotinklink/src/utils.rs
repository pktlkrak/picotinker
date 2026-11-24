pub fn set_u32(arr: &mut [u8], base: usize, new: u32) {
    arr[base..base + 4usize].copy_from_slice(&new.to_le_bytes());
}

pub fn get_u32(arr: &[u8], base: usize) -> u32 {
    ((arr[base + 0] as u32) << 0)
        | ((arr[base + 1] as u32) << 8)
        | ((arr[base + 2] as u32) << 16)
        | ((arr[base + 3] as u32) << 24)
}
