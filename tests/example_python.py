import HUI

c = 0 
def click_counter():
    global c
    c += 1
    return c

w = HUI.WebView()

# FIXME: fixes for Qt5 because it uses old browser
#w.call_js("document.body.appendChild(document.createElement('p')); document.body.appendChild(document.createElement('button'));")

w.html_element("html body p")  # create element
w.html_element("html body p", "innerHTML", "'Hello world! <br> This is python HUI example. <br> <i>The button bellow was clicked 0 times.</i>'")  # edit innerHTML of the element

w.html_element("html body button")  # create element
w.html_element("html body button", "innerHTML", "'Click me!'")  # edit innerHTML of the element
w.html_element("html body button", "onclick", w.call_native(lambda x: w.html_element("html body p", "innerHTML", "'Hello world! <br> This is python HUI example. <br> <i>The button bellow was clicked "+str( click_counter() )+" times.</i>'")) )  # add callback

HUI.WebView.handle_forever()  # start endless message loop
#for f in range(10**6): HUI.WebView.handle_once()  # usefull in interactive shell when you need to keep the message loop running just for some time