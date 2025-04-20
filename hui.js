R"htuijsstr( 

//<script>


////////  HUI enhancements and fixes  ////////

setTimeout(function(){
	function all_elements () {
		
		document.querySelectorAll("input[type='number']").forEach(input => { input.setAttribute("onmousewheel", ""); });  // allow changing vales of input type number using mousewheel (changing values using arrow keys works by default)
		
		document.querySelectorAll("select").forEach(select => { select.onmousewheel= function(e) { // allow changing vales of select using mousewheel (changing values using arrow kes works by default)
			if (this.hasFocus) {
				return;
			}
			if (e.deltaY < 0) {
				this.selectedIndex = Math.max(this.selectedIndex - 1, 0);
			}
			if (e.deltaY > 0) {
				this.selectedIndex = Math.min(this.selectedIndex + 1, this.length - 1);
			}
		}; });
		
	}

	all_elements();
	const observer = new MutationObserver(function(){ all_elements(); });
	observer.observe(document.body, { childList: true, subtree: true });

}, 1000);

// disable pinch zoom in gtk webkit (because it doesnt respect css or meta tags)
if (window.webkit) {
    document.addEventListener('touchstart', function (event) { if(event.scale !== 1 && event.touches.length === 2){event.preventDefault();} }, {passive: false});
    document.addEventListener('touchmove',  function (event) { if(event.scale !== 1 && event.touches.length === 2){event.preventDefault();} }, {passive: false});
}

// disable right-click
document.addEventListener('contextmenu', event => event.preventDefault());

////////  HUI api ////////

// support forEach in webkit qt5
//if(!Array.prototype.forEach){Array.prototype.forEach=function(callback){if(this==null)throw new TypeError("Array.prototype.forEach called on null or undefined");var O=Object(this),len=O.length>>>0;if(typeof callback!=="function")throw new TypeError(callback+" is not a function");var T=arguments.length>1?arguments[1]:void 0;for(var i=0;i<len;i++){if(i in O){callback.call(T,O[i],i,O);}}};} if(window.NodeList && !NodeList.prototype.forEach){NodeList.prototype.forEach = Array.prototype.forEach;}

class _HUI {
	/*_call_cpp (value){
		// extended can have some datacolecting here
		window.webkit.messageHandlers["sent2cpp"].postMessage(value);
	}
	_call_js (code){
		// optionaly used (can impact performance but better use it)
		// its better because we can pass multiple commands and will get return of the last one
		return eval(code);
	}*/
}
var HUI = new _HUI; // DEPRECAED JSERROR{duplicate}

HUI.html_element_create = function (query) {
    
		const queryParts = query.split(/\s+(?![^\[]*\])/); // Split by space but ignore spaces inside attribute selectors
		const elementQuery = queryParts.pop(); // The last part is the element to create
		const parentQuery = queryParts.join(' '); // The remaining part is the parent query

		const tag = elementQuery.match(/^[a-z0-9]+/i)?.[0] ?? 'div'; // Get the tag name, default to div if no tag name
		const ids = elementQuery.match(/#([a-zA-Z0-9\-_]+)/); // Find ID
		const classes = elementQuery.match(/\.[a-zA-Z0-9\-_]+/g); // Find all classes
		const attributes = elementQuery.match(/\[([a-zA-Z\-]+)(?:=['"]?([a-zA-Z0-9\-_ ]*)['"]?)?\]/g);

		const element = document.createElement(tag);
		if (ids) {
			element.id = ids[1];
		}
		if (classes) {
			const classList = classes.map(cls => cls.substring(1)); // Remove the leading '.'
			element.classList.add(...classList);
		}
		if (attributes) {
			attributes.forEach(attr => {
				const attrMatch = attr.match(/\[([a-zA-Z\-]+)(?:=['"]?([a-zA-Z0-9\-_ ]*)['"]?)?\]/);
				const attrName = attrMatch[1]; // Attribute name
				const attrValue = attrMatch[2] || ''; // Attribute value (default is empty string if not provided)
				element.setAttribute(attrName, attrValue);
			});
		}
		
		let parent = document.querySelector(parentQuery);
		if (parent) {
			parent.appendChild(element);
		}
		else {
			//createElementFromQuery(parent);
			console.error("<hui_error: invalid parent '"+parentQuery+"' of new element>");
		}

		return element;
		
	}


/*class HUIbutton extends HTMLElement {
  constructor() {
    super();

    const shadow = this.attachShadow({mode: 'open'});
	
	/.*<style>
	span {}
	span:hover, span:focus {}
	span:disabled {}
	</style>*./
	
	shadow.appendChild( Object.assign(document.createElement("span"), {innerHTML:"Click me not?",tabindex:"0"}) );
	//<div tabindex="0" contenteditable="true">abc</div>
	// style: ::hover = ::focus
	
	//shadow.appendChild()
	
  }
}
customElements.define('hui-button', HUIbutton);*/

)htuijsstr"
