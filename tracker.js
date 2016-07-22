// Global vars
var canvas = null;
var ctx = null;
var frames = 1;
var startTime = 0;

var theData = null;

var requestAnimationFrame =
    requestAnimationFrame ||
    webkitRequestAnimationFrame ||
    mozRequestAnimationFrame ||
    msRequestAnimationFrame ||
    oRequestAnimationFrame;

function requestData() {
    $.getJSON( "sounds.json", function( data ) {
	fireloop(data);
    });
}

function fireloop(data) {
    theData = data;
    setTimeout(requestData,1000.0/60); //60 fps target
}

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

// ----------------------------------------
window.onload = function () {
    canvas = document.getElementById("screen");
    ctx = canvas.getContext("2d");

    startTime = performance.now();
    requestData();
    requestAnimationFrame(loop);
};
