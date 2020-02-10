use renc_sys::*;
use std::ffi::CString;
use std::os::raw::c_void;

fn main () {
    unsafe {
        rebStartup();
        //assert_eq!(0, RL_rebTick());
        let one: *mut Reb_Value = RL_rebInteger(1i64);
        println!("one: {}", one as u64);
        //println!("one: {}", echo);
        //assert_eq!(1, echo);
        //assert_eq!(1, RL_rebUnboxInteger0(one as *const c_void));
        rebRelease(one);
        rebShutdown(true);

        RL_rebStartup();
        let one: *mut Reb_Value = rebInteger(1i64);
        let reb_end: [u8;2] = [0x80, 0x00];

        let expr = CString::new("1 +").unwrap();
        let two: *mut Reb_Value = rebValue(expr.as_ptr() as *const c_void, one as *const c_void, reb_end.as_ptr());
        assert_eq!(2, rebUnboxInteger(two as *const c_void, reb_end.as_ptr()));
        rebRelease(two);
        rebRelease(one);

        RL_rebShutdown(true);

        rebStartup();
        rebShutdown(true);
    }
}
