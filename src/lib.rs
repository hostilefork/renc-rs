use std::ffi::{CString, CStr, c_void};
use std::marker::PhantomData;
use std::sync::atomic::{AtomicBool, Ordering};

use failure::Fail;
use log::trace;

#[derive(Debug)]
pub struct RebEngine ();

#[derive(Debug)]
pub struct RebValue<'a> {
    inner: *mut renc_sys::Reb_Value,
    engine: &'a RebEngine,
}

#[derive(Debug)]
pub struct RebErrorValue<'a> {
        type_: String,
        id: String,
        message: String,
    inner: *mut renc_sys::Reb_Value,
    engine: PhantomData<&'a RebEngine>,
}

pub enum RebErrorType {
    Math,
}

#[derive(Debug, Fail)]
pub enum RebError {
    #[fail(display = "Rebol Error, type: {}, id: {}, message: {}", type_, id, message)]
    RebError {
        //type_: RebErrorType,
        type_: String,
        id: String,
        message: String,
        near: String,
        where_: String,
        file: String,
        line: u32,
    }
}

impl RebError {
    fn from_rebval(e: &RebValue) -> Self {
        RebError::RebError {
            type_: e.engine.map_field(e, "type", |v| unsafe {v.unbox_string_q()}),
            id: e.engine.map_field(e, "id", |v| unsafe {v.unbox_string_q()}),
            message: e.engine.map_field(e, "message", |v| unsafe {v.unbox_string()}),
            near: String::new(),
            where_: String::new(),
            file: e.engine.map_field(e, "file", |v| unsafe {v.unbox_string_q()}),
            line: 0,
        }
    }
}

pub trait RebCode {
    fn as_const_ptr(&self) -> *const c_void;
}

static REB_END: [u8; 2] = [0x80, 0x00];
static REB_STARTED_UP: AtomicBool = AtomicBool::new(false);

#[repr(transparent)]
pub struct CUtf8 {
    s: CString,
}

impl CUtf8 {
    pub fn new(s: &str) -> Self {
        Self {
            s: CString::new(s).unwrap(),
        }
    }
}

impl<'a, 'b> RebEngine {
    pub fn new() -> Self {
        if REB_STARTED_UP.compare_and_swap(false, true, Ordering::SeqCst) {
            panic!("Another thread is already running the renc engine");
        }
        unsafe{renc_sys::rebStartup();}
        Self {}
    }

    pub fn tick(&self) -> usize {
        unsafe {renc_sys::rebTick()}
    }

    pub fn void(&self) -> RebValue {
        unsafe {
            RebValue::from_raw(self,
                renc_sys::rebVoid())
        }
    }

    pub fn blank(&self) -> RebValue {
        unsafe {
            RebValue::from_raw(self,
                renc_sys::rebBlank())
        }
    }

    pub fn char(&self, v: char) -> RebValue {
        unsafe {
            RebValue::from_raw(self,
                renc_sys::rebChar(v as u32))
        }
    }

    pub fn integer(&self, v: i64) -> RebValue {
        unsafe {
            RebValue::from_raw(self,
                renc_sys::rebInteger(v))
        }
    }

    pub fn decimal(&self, v: f64) -> RebValue {
        unsafe {
            RebValue::from_raw(self,
                renc_sys::rebDecimal(v))
        }
    }

    /*
    pub fn sized_binary<'a, 'b, T: Into<&'b [u8]>>(&'a self, v: T, len: usize) -> RebValue {
        unsafe {
            RebValue::from_raw(self,
                               renc_sys::rebSizedBinary(v.into().as_ptr() as *const c_void, len))
        }
    }
    */

    pub fn load(&self, code: &str) -> Result<RebValue, RebError>
    {
        let c = CUtf8::new(code);
        let v = unsafe {renc_sys::rebValueQ(c.as_const_ptr(),
                REB_END.as_ptr())};

        let is_error =  unsafe {
            renc_sys::rebDid(CUtf8::new("error?").as_const_ptr(),
                v,
                REB_END.as_ptr())
        };
        if is_error {
            Err(RebError::from_rebval(unsafe {
                &RebValue::from_raw(self, v)
            }))
        } else {
            unsafe {
                Ok(RebValue::from_raw(self, v))
            }
        }
    }

    pub fn map_field<F, G>(&self, a: &RebValue, field: &str, f: F) -> G
    where
        F: Fn(&RebValue) -> G
    {
        let v = unsafe {RebValue::from_raw(self,
                            renc_sys::rebValue(
                                CUtf8::new("get in").as_const_ptr(),
                                a.inner,
                                CUtf8::new(&format!("'{}", field)).as_const_ptr(),
                                REB_END.as_ptr())
                            )};
        f(&v)
    }

    pub fn value1<A>(&'a self, a: &'b A) -> Result<RebValue<'a>, RebError>
    where
        A: RebCode
    {
        /*
        let v = unsafe {renc_sys::rebValueQ(a.as_const_ptr(),
                REB_END.as_ptr())};

        let is_error =  unsafe {
            renc_sys::rebDid(CUtf8::new("error?").as_const_ptr(),
                v,
                REB_END.as_ptr())
        };
        if is_error {
            return Err(RebError::from_rebval(unsafe {
                &RebValue::from_raw(self, v)
            }));
        }
        */

        let trapped = unsafe {
            renc_sys::rebValue(
                CUtf8::new("entrap [").as_const_ptr(),
                a.as_const_ptr(),
                CUtf8::new("]").as_const_ptr(),
                REB_END.as_ptr())
        };

        //unsafe {renc_sys::rebRelease(v);}
        /*
        unsafe {
            renc_sys::rebElide(
                CUtf8::new("print mold").as_const_ptr(),
                trapped,
                REB_END.as_ptr());
        }
        */
        let is_error = unsafe {
            renc_sys::rebDid(
                CUtf8::new("error?").as_const_ptr(),
                trapped,
                REB_END.as_ptr())
        };
        if is_error {
            /*
            unsafe {
                renc_sys::rebElide(
                    CUtf8::new("print mold").as_const_ptr(),
                    trapped,
                    REB_END.as_ptr());
            }
            */

            let trapped = unsafe {RebValue::from_raw(self, trapped)};

            let e = RebError::from_rebval(&trapped);
            return Err(e);
        } else {
            let is_block = unsafe {
                renc_sys::rebDid(
                    CUtf8::new("block?").as_const_ptr(),
                    trapped,
                    REB_END.as_ptr())
            };
            if is_block {
                let inner = unsafe {
                    renc_sys::rebValue(
                        CUtf8::new("first").as_const_ptr(),
                        trapped,
                        REB_END.as_ptr())
                };
                unsafe {
                    renc_sys::rebRelease(trapped);
                }
                Ok(RebValue {
                    inner,
                    engine: self
                })
            } else {
                Ok(RebValue {
                    inner: trapped,
                    engine: self
                })
            }
        }
    }

    pub fn value2<A, B>(&self, a: &A, b: &B) -> Result<RebValue, RebValue>
    where
        A: RebCode,
        B: RebCode,
    {
        let entrap = CUtf8::new("entrap [");
        let bracket = CUtf8::new("]");
        let trapped = unsafe {
            renc_sys::rebValue(entrap.as_const_ptr(),
                a.as_const_ptr(),
                b.as_const_ptr(),
                bracket.as_const_ptr(),
                REB_END.as_ptr())
        };
        let error_check = CUtf8::new("error?");
        let is_error = unsafe {
            renc_sys::rebDid(error_check.as_const_ptr(),
                trapped,
                REB_END.as_ptr())
        };
        if is_error {
            Err(RebValue {
                inner: trapped,
                engine: self
            })
        } else {
            let block_check = CUtf8::new("block?");
            let is_block = unsafe {
                renc_sys::rebDid(block_check.as_const_ptr(),
                    trapped,
                    REB_END.as_ptr())
            };
            if is_block {
                let first = CUtf8::new("first");
                let inner = unsafe {
                    renc_sys::rebValue(first.as_const_ptr(),
                        trapped,
                        REB_END.as_ptr())
                };
                unsafe {
                    renc_sys::rebRelease(trapped);
                }
                Ok(RebValue {
                    inner,
                    engine: self
                })
            } else {
                Ok(RebValue {
                    inner: trapped,
                    engine: self
                })
            }
        }
    }

    pub fn value3<A, B, C>(&self, a: &A, b: &B, c: &C) -> RebValue
    where
        A: RebCode,
        B: RebCode,
        C: RebCode,
    {
        let inner = unsafe {renc_sys::rebValue(a.as_const_ptr(),
                                               b.as_const_ptr(),
                                               c.as_const_ptr(),
                                               REB_END.as_ptr())};
        RebValue {
            inner,
            engine: self
        }
    }

    pub fn elide<T: RebCode>(&self, t: &T) {
        unsafe {renc_sys::rebElide(t.as_const_ptr(), REB_END.as_ptr())};
    }
}

impl<'a> RebValue<'a> {
    pub fn unbox_integer(&self) -> isize {
        unsafe {renc_sys::rebUnboxInteger(self.inner as *const c_void, REB_END.as_ptr())}
    }
    pub unsafe fn unbox_string(&self) -> String {
        let c = renc_sys::rebSpell(self.inner as *const c_void, REB_END.as_ptr());
        let r = CStr::from_ptr(c).to_str().unwrap().to_owned();
        //println!("c: {:?}", c);
        renc_sys::rebFree(c as *mut c_void);
        r
    }
    pub unsafe fn unbox_string_q(&self) -> String {
        let c = renc_sys::rebSpellQ(self.inner as *const c_void, REB_END.as_ptr());
        let r = CStr::from_ptr(c).to_str().unwrap().to_owned();
        //println!("c: {:?}", c);
        renc_sys::rebFree(c as *mut c_void);
        r
    }
    unsafe fn from_raw(engine: &'a RebEngine, inner: *mut renc_sys::Reb_Value) -> Self
    {
        Self {
            inner, 
            engine,
        }
    }
}


impl<'a> Drop for RebValue<'a> {
    fn drop(&mut self) {
        trace!("dropping a rebval");
        unsafe{renc_sys::rebRelease(self.inner);}
    }
}

impl<'a> RebCode for RebValue<'a> {
    fn as_const_ptr(&self) ->*const c_void {
        assert!(!self.inner.is_null());
        self.inner as *const c_void
    }
}

impl RebCode for CUtf8 {
    fn as_const_ptr(&self) -> *const c_void {
        self.s.as_ptr() as *const c_void
    }
}

impl Drop for RebEngine {
    fn drop(&mut self) {
        trace!("dropping a rebengine");
        unsafe{renc_sys::rebShutdown(true);}
        if ! REB_STARTED_UP.swap(false, Ordering::SeqCst) {
            panic!("Renc engine is not running in this thread");
        }
    }
}

/*
macro_rule! evaluate {
    ($engine:expr, $($arg:expr),+) => {
        renc_sys::rebValue($($arg),+, REB_END.as_ptr())
    }
}
*/

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn engine() {
        let _engine = RebEngine::new();
    }

    #[test]
    fn unbox() {
        let engine = RebEngine::new();
        let one = engine.integer(1);
        assert_eq!(1, one.unbox_integer());
    }

    #[test]
    fn one_plus_one_v0() {
        let engine = RebEngine::new();
        let two = match engine.value1(&CUtf8::new("1 + 1")) {
            Ok(v) => v,
            Err(e) => {
                println!("Failed: {:?}", e);
                return;
            }
        };
        assert_eq!(2, two.unbox_integer());
    }

    #[test]
    fn one_plus_one_v1() {
        let engine = RebEngine::new();
        let two = engine.value2(&CUtf8::new("1 + "), &engine.integer(1)).unwrap();
        assert_eq!(2, two.unbox_integer());
    }

    #[test]
    fn one_plus_one_v2() {
        let engine = RebEngine::new();
        let one = engine.integer(1);
        let two = engine.value3(&one, &CUtf8::new("+"), &one);
        assert_eq!(2, two.unbox_integer());
    }

    #[test]
    fn hello_world() {
        let engine = RebEngine::new();
        engine.elide(&CUtf8::new(r##"print "hello, world!""##));
    }

    #[test]
    fn func_call() {
        let engine = RebEngine::new();
        let fib_str = CUtf8::new(r##"
        func[
            n [integer!]
        ][
            if n <= 1 [return n]
            f0: 0
            f1: 1
            for i 2 n 1 [
                f: f0 + f1
                f0: f1
                f1: f
            ]
            f
        ]"##);
        //dbg!(&fib_str);
        let fib = engine.value1(&fib_str).unwrap();
        //dbg!(&fib);
        let fibn = engine.value2(&fib, &engine.integer(0)).unwrap();
        assert_eq!(0, fibn.unbox_integer());

        let fibn = engine.value2(&fib, &engine.integer(1)).unwrap();
        assert_eq!(1, fibn.unbox_integer());

        let fibn = engine.value2(&fib, &engine.integer(2)).unwrap();
        assert_eq!(1, fibn.unbox_integer());

        let fibn = engine.value2(&fib, &engine.integer(3)).unwrap();
        assert_eq!(2, fibn.unbox_integer());

        let fibn = engine.value2(&fib, &engine.integer(4)).unwrap();
        assert_eq!(3, fibn.unbox_integer());

        let fibn = engine.value2(&fib, &engine.integer(5)).unwrap();
        assert_eq!(5, fibn.unbox_integer());

        let fibn = engine.value2(&fib, &engine.integer(6)).unwrap();
        assert_eq!(8, fibn.unbox_integer());

        let fibn = engine.value2(&fib, &engine.integer(7)).unwrap();
        assert_eq!(13, fibn.unbox_integer());

        let fibn = engine.value2(&fib, &engine.integer(8)).unwrap();
        assert_eq!(21, fibn.unbox_integer());

        let fibn = engine.value2(&fib, &engine.integer(9)).unwrap();
        assert_eq!(34, fibn.unbox_integer());

        let fibn = engine.value2(&fib, &engine.integer(10)).unwrap();
        assert_eq!(55, fibn.unbox_integer());
    }

    #[test]
    fn hello_error() {
        let engine = RebEngine::new();
        let e = engine.value1(&CUtf8::new("1 / 0"));
        //println!("e: {:?}", e);
        assert!(e.is_err());
    }
}
