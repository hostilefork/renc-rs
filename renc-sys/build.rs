use std::env;
use std::path::PathBuf;

fn main() {
    build_shim();

    // Tell cargo to tell rustc to link the system bzip2
    // shared library.
    #[cfg(target_os = "windows")]
    println!("cargo:rustc-link-lib=libr3");

    #[cfg(target_os = "linux")]
    println!("cargo:rustc-link-lib=r3");

    println!("cargo:rustc-link-search=native=renc/lib");

    gen_binding();
}

fn gen_binding()
{
    use bindgen::Builder;
    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header("wrapper.h")
        //.clang_arg("-Irenc/include")
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    println!("binding is at: {:?}", &out_path);
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}

fn build_shim()
{
    use cc::Build;
    Build::new()
        .file("renc/shim/valist.c")
        //.define("REBOL_EXPLICIT_END")
        .pic(true) // shared library requires this flag
        .shared_flag(true)
        .compile("r3shim");
}
