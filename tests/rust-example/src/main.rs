use std::sync::Arc;
use HUI::*;


fn button_callback(_args: Vec<String>, window: Arc<WebView>) {
    window.call_js("document.querySelector('button').hidden = false;", None);
    println!("button shown");
    //println!("args: {:?}", args);
}

fn main() {
    // wrap the WebView in an Arc to allow shared ownership
    let window = Arc::new(WebView::new());

	// load html
    let html = r#"<!DOCTYPE html>
<html>
  <head>
  </head>
  <body>
    <button>click me</button>
  </body>
<html>"#;

    window.load_str(html);


    let window_clone = Arc::clone(&window);

    window.html_element(
        "body",
        "oncontextmenu",
        &window.call_native(
            move |args| button_callback(args, Arc::clone(&window_clone)),
            None,
        ),
    );
	

    let window_for_closure = Arc::clone(&window);

    window.html_element(
        "button",
        "onclick",
        &window.call_native(
            // use 'move' to capture the cloned Arc by value
            move |_args| {
                window_for_closure
                    .call_js("document.querySelector('button').hidden = true;", None);
                println!("button removed");
            },
			None
        ),
    );
	
	
	window.call_js_async("document.body; document.body.innerText", move |value| { println!("async return value: {}", value); } );


    WebView::handle_forever();
}