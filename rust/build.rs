fn main() {

    // set library (HUI build) directory
    println!("cargo:rustc-link-search=native={}", "../build");
	
	// deal with lib prefix weirdness
    if cfg!(target_os = "windows") {
        println!("cargo:rustc-link-lib=dylib=libHUI");
    } else {
        println!("cargo:rustc-link-lib=dylib=HUI");
    }

}