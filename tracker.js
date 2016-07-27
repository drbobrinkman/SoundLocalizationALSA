/*! \file tracker.js
 * 2D view of sound tracker data from the server, for debugging purposes.
 *
 * \author Bo Brinkman <dr.bo.brinkman@gmail.com>
 * \date 2016-07-27
 */

/*
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **/

/*! For convenient access to canvas element */
var canvas = null;
/*! For convenient access to the drawing context */
var ctx = null;

/*! Frame rate counter. \bug Currently not handling possibility of overflow */
var frames = 1;
/*! Start time, for calculating frame rate */
var startTime = 0;

/*! Global so the data request loop can provide data to the draw loop.
 *
 * This assumes a single Javascript thread, so no race conditions.
 */
var theData = null;

/*! For compatibility with older browseres */
var requestAnimationFrame =
    requestAnimationFrame ||
    webkitRequestAnimationFrame ||
    mozRequestAnimationFrame ||
    msRequestAnimationFrame ||
    oRequestAnimationFrame;

/*! Requests data from the server */
function requestData() {
    $.getJSON( "sounds.json", function( data ) {
	fireloop(data);
    });
}

/*! Handles data from the server, then sets another data request
 *  to fire for next frame */
function fireloop(data) {
    theData = data;
    setTimeout(requestData,1000.0/60); //60 fps target
}

/*! Main draw loop, which fires independently of the data request loop */
function loop() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    curTime = performance.now();
    elapsed = curTime - startTime;

    
    if(theData != null){
	curFrame = theData["current_frame"];
	
	//First, draw the mic setup
	ctx.fillStyle = "#007f00";
	ctx.beginPath();
	sideLength = 50;
	rt3 = Math.sqrt(3);
	ctx.moveTo(canvas.width/2, canvas.height/2 - sideLength/rt3);
	ctx.lineTo(canvas.width/2 + sideLength/2,
		   canvas.height/2 + sideLength/(2*rt3));
	ctx.lineTo(canvas.width/2 - sideLength/2,
		   canvas.height/2 + sideLength/(2*rt3));
	ctx.closePath();
	ctx.fill();  

	ctx.font = "12px Courier";
	ctx.fillText("0", canvas.width/2 - 4,
		     canvas.height/2 - sideLength/rt3 - 4);
	
	
	$.each( theData["sounds"], function( key, val ) {
	    x = val["location"][0];
	    y = val["location"][1];
	    size = Math.sqrt(val["loudness"]);

	    x = canvas.width/2 + x*(canvas.width/4);
	    y = canvas.height/2 - y*(canvas.width/4); //Invert y axis

	    fadeAmt = (curFrame - val["last_frame"])/10.0; //one sixth of a second
	    ctx.fillStyle = "rgba(0, 0, 0, "+ (1-fadeAmt) +")";
	    ctx.fillRect(x-size/2, y-size/2, size, size);

	});
    }

    ctx.font = "30px Arial";
    ctx.fillStyle = "black";
    ctx.fillText("fps: " + Math.round(frames*1000/elapsed), 5,
                 canvas.height-5);

    ++frames;
    requestAnimationFrame(loop);
}

/*! Initialize the global variables and start the two loops */
window.onload = function () {
    canvas = document.getElementById("screen");
    ctx = canvas.getContext("2d");

    startTime = performance.now();
    requestData();
    requestAnimationFrame(loop);
};
