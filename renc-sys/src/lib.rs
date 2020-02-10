#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

#[cfg(test)]
mod tests {
    use super::*;
    //use std::mem;
    use std::os::raw::c_void;
    use std::ffi::CString;

    #[test]
    fn startup () {
        unsafe {
            rebStartup();
            let one: *mut Reb_Value = rebInteger(1i64);
            let rebEnd: [u8;2] = [0x80, 0x00];
            assert_eq!(1, rebUnboxInteger(one as *const c_void, rebEnd.as_ptr()));
            rebRelease(one);

            rebShutdown(true);
        }
    }

    #[test]
    fn one_plus_one() {
        unsafe {
            RL_rebStartup();
            let one: *mut Reb_Value = rebInteger(1i64);
            let rebEnd: [u8;2] = [0x80, 0x00];

            let expr = CString::new("1 +").unwrap();
            let two: *mut Reb_Value = rebValue(expr.as_ptr() as *const c_void, one as *const c_void, rebEnd.as_ptr());
            assert_eq!(2, rebUnboxInteger(two as *const c_void, rebEnd.as_ptr()));
            rebRelease(two);
            rebRelease(one);

            RL_rebShutdown(true);
        }
    }
}
