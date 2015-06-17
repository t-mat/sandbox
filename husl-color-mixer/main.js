$(document).ready(function() {

// http://stackoverflow.com/a/5624139/2132223
function toInt(v) {
	return ~~v;
}

function componentToHex(c) {
	var hex = toInt(c).toString(16);
	return hex.length == 1 ? "0" + hex : hex;
}

function rgbToHex(rgb) {
	return "#" + componentToHex(rgb.r) + componentToHex(rgb.g) + componentToHex(rgb.b);
}

function hexToRgb(hex) {
	// Expand shorthand form (e.g. "03F") to full form (e.g. "0033FF")
	var shorthandRegex = /^#?([a-f\d])([a-f\d])([a-f\d])$/i;
	hex = hex.replace(shorthandRegex, function(m, r, g, b) {
		return r + r + g + g + b + b;
	});

	var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
	return result ? {
		r: parseInt(result[1], 16),
		g: parseInt(result[2], 16),
		b: parseInt(result[3], 16)
	} : null;
}

function hueInterpolator(h1, h2, t) {
	var dh = (h2 - h1) % 1.0;
	if(Math.abs(dh) > 0.5) {
		if(dh > 0) {
			dh = dh - 1.0;
		} else {
			dh = dh + 1.0;
		}
	}
	var h = h1 + dh * t;
	while(h > 1.0) {
		h -= 1.0;
	}
	while(h < 0.0) {
		h += 1.0;
	}
	return h;
}

// RGB
function rgbInterpolator(rgb1, rgb2, t, e) {
	var fl = 0;
	var s = 1.0 - t;
	var r = rgb1.r * s + rgb2.r * t;
	var g = rgb1.g * s + rgb2.g * t;
	var b = rgb1.b * s + rgb2.b * t;
//	$(e).text("r="+r.toFixed(fl)+", g="+g.toFixed(fl)+", b="+b.toFixed(fl));
	return { r:r, g:g, b:b };
}

// HSL
// http://axonflux.com/handy-rgb-to-hsl-and-rgb-to-hsv-color-model-c
/**
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param	Number	h		The hue
 * @param	Number	s		The saturation
 * @param	Number	l		The lightness
 * @return	Array			The RGB representation
 */
function hslToRgb_(h, s, l){
	var r, g, b;

	if(s == 0){
		r = g = b = l; // achromatic
	}else{
		var hue2rgb = function hue2rgb(p, q, t){
			if(t < 0) t += 1;
			if(t > 1) t -= 1;
			if(t < 1/6) return p + (q - p) * 6 * t;
			if(t < 1/2) return q;
			if(t < 2/3) return p + (q - p) * (2/3 - t) * 6;
			return p;
		}

		var q = l < 0.5 ? l * (1 + s) : l + s - l * s;
		var p = 2 * l - q;
		r = hue2rgb(p, q, h + 1/3);
		g = hue2rgb(p, q, h);
		b = hue2rgb(p, q, h - 1/3);
	}

	return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
}

/**
 * Converts an RGB color value to HSL. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes r, g, and b are contained in the set [0, 255] and
 * returns h, s, and l in the set [0, 1].
 *
 * @param	Number	r		The red color value
 * @param	Number	g		The green color value
 * @param	Number	b		The blue color value
 * @return	Array			The HSL representation
 */
function rgbToHsl_(r, g, b){
	r /= 255, g /= 255, b /= 255;
	var max = Math.max(r, g, b), min = Math.min(r, g, b);
	var h, s, l = (max + min) / 2;

	if(max == min){
		h = s = 0; // achromatic
	}else{
		var d = max - min;
		s = l > 0.5 ? d / (2 - max - min) : d / (max + min);
		switch(max){
			case r: h = (g - b) / d + (g < b ? 6 : 0); break;
			case g: h = (b - r) / d + 2; break;
			case b: h = (r - g) / d + 4; break;
		}
		h /= 6;
	}

	return [h, s, l];
}


/**
 * Converts an RGB color value to HSV. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes r, g, and b are contained in the set [0, 255] and
 * returns h, s, and v in the set [0, 1].
 *
 * @param	Number	r		The red color value
 * @param	Number	g		The green color value
 * @param	Number	b		The blue color value
 * @return	Array			The HSV representation
 */
function rgbToHsv_(r, g, b){
	r = r/255, g = g/255, b = b/255;
	var max = Math.max(r, g, b), min = Math.min(r, g, b);
	var h, s, v = max;

	var d = max - min;
	s = max == 0 ? 0 : d / max;

	if(max == min){
		h = 0; // achromatic
	}else{
		switch(max){
			case r: h = (g - b) / d + (g < b ? 6 : 0); break;
			case g: h = (b - r) / d + 2; break;
			case b: h = (r - g) / d + 4; break;
		}
		h /= 6;
	}

	return [h, s, v];
}

/**
 * Converts an HSV color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes h, s, and v are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param	Number	h		The hue
 * @param	Number	s		The saturation
 * @param	Number	v		The value
 * @return	Array			The RGB representation
 */
function hsvToRgb_(h, s, v){
	var r, g, b;

	var i = Math.floor(h * 6);
	var f = h * 6 - i;
	var p = v * (1 - s);
	var q = v * (1 - f * s);
	var t = v * (1 - (1 - f) * s);

	switch(i % 6){
		case 0: r = v, g = t, b = p; break;
		case 1: r = q, g = v, b = p; break;
		case 2: r = p, g = v, b = t; break;
		case 3: r = p, g = q, b = v; break;
		case 4: r = t, g = p, b = v; break;
		case 5: r = v, g = p, b = q; break;
	}

	return [r * 255, g * 255, b * 255];
}




function hslInterpolator(rgb1, rgb2, t, e) {
	var t1 = 1.0 - t;
	var t2 = t;
	var hsl1 = rgbToHsl_(rgb1.r, rgb1.g, rgb1.b);
	var hsl2 = rgbToHsl_(rgb2.r, rgb2.g, rgb2.b);

	var h = hueInterpolator(hsl1[0], hsl2[0], t2);
	var s = hsl1[1] * t1 + hsl2[1] * t2;
	var l = hsl1[2] * t1 + hsl2[2] * t2;

	var rgb = hslToRgb_(h, s, l);
	return { r:rgb[0], g:rgb[1], b:rgb[2] };
}

function hsvInterpolator(rgb1, rgb2, t, e) {
	var t1 = 1.0 - t;
	var t2 = t;
	var hsv1 = rgbToHsv_(rgb1.r, rgb1.g, rgb1.b);
	var hsv2 = rgbToHsv_(rgb2.r, rgb2.g, rgb2.b);

	var h = hueInterpolator(hsv1[0], hsv2[0], t2);
	var s = hsv1[1] * t1 + hsv2[1] * t2;
	var v = hsv1[2] * t1 + hsv2[2] * t2;

	var rgb = hsvToRgb_(h, s, v);
	return { r:rgb[0], g:rgb[1], b:rgb[2] };
}

// HUSL
function rgbToHusl_(r, g, b) {
	var hsl = $.husl.fromRGB(r/255.0, g/255.0, b/255.0);
	var h = hsl[0] / 360.0;
	var s = hsl[1] / 100.0;
	var l = hsl[2] / 100.0;
	return [ h, s, l ];
}

function huslToRgb_(h, s, l) {
	var rgb = $.husl.toRGB(h * 360.0, s * 100.0, l * 100.0);
	return [ rgb[0]*255, rgb[1]*255, rgb[2]*255 ];
}

function huslInterpolator(rgb1, rgb2, t, e) {
	var t1 = 1.0 - t;
	var t2 = t;
	var hsl1 = rgbToHusl_(rgb1.r, rgb1.g, rgb1.b);
	var hsl2 = rgbToHusl_(rgb2.r, rgb2.g, rgb2.b);

	var h = hueInterpolator(hsl1[0], hsl2[0], t2);
	var s = hsl1[1] * t1 + hsl2[1] * t2;
	var l = hsl1[2] * t1 + hsl2[2] * t2;

//	$(e).text("h="+h.toFixed(2)+", s="+s.toFixed(2)+", l="+l.toFixed(2));

	var rgb = huslToRgb_(h, s, l);
	return { r:rgb[0], g:rgb[1], b:rgb[2] };
}

function update1(id, hexColor1, hexColor2, interpolatorFunc) {
	var els = $(id).nextAll();
	var col1 = hexToRgb(hexColor1);
	var col2 = hexToRgb(hexColor2);
	els.each(function(index) {
		var t = index / (els.length - 1);
		$(this).text(toInt(t*100) + "%");
		var c = interpolatorFunc(col1, col2, t, this);
		$(this).css("background-color", rgbToHex(c));
	});
}

function update() {
	color1 = $("#color1").val();
	color2 = $("#color2").val();
	update1("#husl", color1, color2, huslInterpolator);
	update1("#hsl" , color1, color2, hslInterpolator);
	update1("#hsv" , color1, color2, hsvInterpolator);
	update1("#rgb" , color1, color2, rgbInterpolator);
}

function changeInputColor(e) {
	update();
}

color1 = "#ffff00";
color2 = "#ff00ff";

$("input#color1").val(color1);
$("input#color2").val(color2);
$("input#color1").on("change", changeInputColor);
$("input#color2").on("change", changeInputColor);

update();

$(".minicolors").minicolors({
	  theme: 'bootstrap'
	, control: 'brightness'
});

});
