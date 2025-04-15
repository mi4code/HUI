#include <iostream>
#include "HUI.hh" // include HUI
// #include "hui_webview__*.cc" // optionaly, you can include the backend code and build it with your app

using namespace HUI;

// TODO: autoload html when compiling from file to definition
const char* ui = R"str( 

<!DOCTYPE html>
<html>
	<body>

		<style>

body {
	padding: 27px;
}

p {
	font-family: Arial, Helvetica, sans-serif, bold;
	color: black;
	line-height: 35px;
	font-size: 16px;
	/*border: 2px black;*/
}

		</style>
		
		<p style="color: red;"> <b>NOT WORKING:</b> run <code>call_js</code> without return before main loop </p>
		<p style="color: red;"> <b>NOT WORKING:</b> run <code>call_js</code> with return before main loop </p>
		<p style="color: red;" onload="var callback_before_checker = setInterval(function(){ if(typeof callback_before === 'function'){document.querySelector('p:nth-of-type(3)').style.color='green'; document.querySelector('p:nth-of-type(3) b').innerText='OK: '; clearInterval(callback_before_checker); } },200);"> <b>NOT WORKING:</b> register callback using <code>call_native</code> and <code>call_js</code> before main loop </p>
		
		<p style="color: red;"> <b>NOT WORKING:</b> run <code>call_js</code> without return while in main loop </p>
		<p style="color: red;"> <b>NOT WORKING:</b> run <code>call_js</code> with return while in main loop </p>
		<p style="color: red;" onload="var callback_after_checker = setInterval(function(){ if(typeof callback_after === 'function'){document.querySelector('p:nth-of-type(6)').style.color='green'; document.querySelector('p:nth-of-type(6) b').innerText='OK: '; clearInterval(callback_after_checker); } },200);"> <b>NOT WORKING:</b> register callback using <code>call_native</code> and <code>call_js</code> while in main loop </p>

		<!--
		<p style="color: red;"> <b>NOT WORKING:</b> feature <code>run()</code> </p>
		<p style="color: green; "> <b>OK:</b> feature <code>run()</code> <i>~100ms</i> </p>
		-->

		<script>

test_variable = "test_value";
document.querySelector('p:nth-of-type(3)').onload();
document.querySelector('p:nth-of-type(6)').onload();

		</script>

	</body>
</html>

)str";


int main (int argc, char* argv[]){
	
	WebView window;
	//window.content("test_hui.html"); // TODO: can test use the tested api?
	window.call_js(std::string()+"document.querySelector('html').innerHTML = `"+ui+"`; eval(document.querySelector('body script').innerText);"); // TODO: .onload methods must be explicitely called from js in the ui file
	window.hui_tweaks(); // apply HUI styling, may crash the app if js api is broken
	
	////////// TESTS BEFORE MAIN LOOP //////////
	
	// run call_js without return before main loop
	window.call_js("document.querySelector('p:nth-of-type(1)').style.color = 'green'; document.querySelector('p:nth-of-type(1) b').innerText = 'OK: ';", false);
	
	// register callback using call_native and call_js before main loop
	// TODO: try to call it
	// TODO: check option to set variable name through html_element
	window.call_js( std::string()+"callback_before = "+window.call_native( [&](std::vector<std::string> args)->void{std::cout<<window.call_js("test_variable",true); window.call_js(std::string()+"hello "+args[0]);} )+";", false );
	
	// run call_js with return before main loop (warning: runs somethig like message loop thus should be ran as the last one although its second in the ui)
	// FIXME: needs to be single statement -> accept more, return the last one
	if ( std::string(window.call_js("test_variable",true)) == std::string("test_value") )  window.call_js("document.querySelector('p:nth-of-type(2)').style.color = 'green'; document.querySelector('p:nth-of-type(2) b').innerText = 'OK: ';", false);
	
	////////////////////////////////////////////
	
	
	////////// TESTS AFTER=INSIDE MAIN LOOP //////////
	
	// TODO: same as above, but wrapped in 
	
	window.call_js(std::string()+"tests_after = "+window.call_native([&](std::vector<std::string> args)->void{
		
		// run call_js without return while in main loop
		window.call_js("document.querySelector('p:nth-of-type(4)').style.color = 'green'; document.querySelector('p:nth-of-type(4) b').innerText = 'OK: ';", false);
	
		// run call_js with return while in main loop
		// FIXME: needs to be single statement -> accept more, return the last one
		if ( std::string(window.call_js("test_variable",true)) == std::string("test_value") )  window.call_js("document.querySelector('p:nth-of-type(5)').style.color = 'green'; document.querySelector('p:nth-of-type(5) b').innerText = 'OK: ';", false);
	
	    // register callback using call_native and call_js while in main loop
		window.call_js( std::string()+"callback_after = "+window.call_native( [&](std::vector<std::string> args)->void{std::cout<<window.call_js("test_variable",true); window.call_js(std::string()+"hello "+args[0]);} )+";", false );
		
	})+";");
	window.call_js("tests_after();");
	
	//////////////////////////////////////////////////

	
	WebView::handle_forever();
	WebView::exit();
	
	std::cout<<"CLEAN EXIT XD \n"; // FIXME: never called

}

// TODO: make AI write same test for C (#include "HUI.h") / <other languages>
// TODO: create tests for html_element (another test?) - since it depends only on call_js it is not necessary to test before and after show	
// TODO: ping-pong benchmark (another test?) - send c++ time to js calculate time there, send js time to c++ and calculate time there then add results to ui
// FIXME = discovered high priority bugs