#![no_std]
#![no_main]

extern crate panic_halt as _;
use riscv_rt::entry;

#[entry]
fn main() -> ! {
    let addr_led = 0xFF20_0000 as *mut u32;
    let addr_sw  = 0xFF20_0040 as *const u32;

    loop {
        unsafe {
            let value = core::ptr::read_volatile(addr_sw);
            core::ptr::write_volatile(addr_led, value);
        }
    }
}