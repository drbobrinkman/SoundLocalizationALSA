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

    curFrame = theData["current_frame"];
    
    curTime = performance.now();
    elapsed = curTime - startTime;

    $.each( theData["sounds"], function( key, val ) {
	ctx.beginPath();
	
	x = val["location"][0];
	y = val["location"][1];
	size = Math.sqrt(val["loudness"]);

	x = canvas.width/2 + x*(canvas.width/4);
	y = canvas.height/2 + y*(canvas.width/4);

	fadeAmt = (curFrame - val["last_frame"])/10.0; //one sixth of a second
	ctx.fillStyle = "rgba(0, 0, 0, "+ (1-fadeAmt) +")";
	ctx.fillRect(x, y, size, size);

    });

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

    ctx.font = "30px Arial";
    
    startTime = performance.now();
    requestData();
    requestAnimationFrame(loop);
};
