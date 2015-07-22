var global;

/////////////////////////////////////////////////////////////////////////////
function random(minVal, maxVal) {
    return Math.random() * (maxVal - minVal) + minVal;
}


// Cubic Bezier curves
// https://en.wikipedia.org/wiki/B%C3%A9zier_curve#Cubic_B.C3.A9zier_curves
function cubicBezier(p0, p1, p2, p3, t) {
    var s = 1 - t;

    var a0 =   s*s*s;
    var a1 = 3*s*s*t;
    var a2 = 3*s*t*t;
    var a3 =   t*t*t;

    var x = a0*p0.x + a1*p1.x + a2*p2.x + a3*p3.x;
    var y = a0*p0.y + a1*p1.y + a2*p2.y + a3*p3.y;

    return { x: x, y: y};
}


/////////////////////////////////////////////////////////////////////////////
function Global(argCanvas) {
    this.intervalId = null;
    this.frameRate  = 30;       // 30 fps
    this.canvas     = argCanvas;
    this.ctx        = argCanvas.getContext('2d');
    this.width      = argCanvas.width;
    this.height     = argCanvas.height;
    this.keyboard   = new Keyboard(window);
    this.dbg        = new DebugLogger(undefined);
    this.entities   = new Entities();
};

Global.prototype.setDebugLoggerElement = function(e) {
    this.dbg = new DebugLogger(e);
};

Global.prototype.init = function(element) {
    this.entities.push(new Player(
          0.5 * this.width
        , 0.8 * this.height
    ));

    for(var i = 0; i < 7; ++i) {
        this.entities.push(new Enemy(
              random(0, this.width)
            , random(0, this.height)
        ));
    }
};

Global.prototype.activate = function(enable) {
    var that = this;
    if(enable) {
        if(! this.intervalId) {
            this.intervalId = setInterval(function() {
                that.update();
                that.render();
            }, 1000 / this.frameRate);
        }
    } else {
        if(this.intervalId) {
            clearInterval(this.intervalId);
            this.intervalId = null;
        }
    }
};

Global.prototype.update = function() {
    this.keyboard.update();
    this.entities.update();
};

Global.prototype.render = function() {
    this.ctx.clearRect(0, 0, this.width, this.height);
    this.entities.render();
};

Global.prototype.line = function(x0, y0, x1, y1, color) {
    var ctx = this.ctx;
    ctx.beginPath();
    if(typeof(color) !== 'undefined') {
        ctx.strokeStyle = color;
    }
    ctx.moveTo(x0, y0);
    ctx.lineTo(x1, y1);
    ctx.stroke();
}


/////////////////////////////////////////////////////////////////////////////
function moveBounce(e) {
    e.x += e.vx;
    e.y += e.vy;

    if(e.x < 0) {
        e.x = 0;
        e.vx = -e.vx;
    } else if(e.x > global.width) {
        e.x = global.width;
        e.vx = -e.vx;
    }

    if(e.y < 0) {
        e.y = 0;
        e.vy = -e.vy;
    } else if(e.y > global.height) {
        e.y = global.height;
        e.vy = -e.vy;
    }
}


/////////////////////////////////////////////////////////////////////////////
function Player(x, y) {
    this.className      = "Player";
    this.x              = x;
    this.y              = y;
    this.vx             = +1.9;
    this.vy             = +2.1;
    this.color          = '#ffffff';
    this.timer          = 30;
    this.shotCounter    = 0;
    this.shotCounterMax = 8;
}

Player.prototype.update = function() {
    moveBounce(this);
    var shotFlag = false;
    if(--this.timer < 0) {
        this.timer = random(0.8, 1.4) * global.frameRate;
        shotFlag = true;
    }
    if(shotFlag) {
        var that = this;
        global.entities.every(function(obj) {
            if(obj.className == 'Enemy') {
                that.shotCounter += 1;
                that.shotCounter %= that.shotCounterMax;
                var a = (that.shotCounter / (that.shotCounterMax-1)) * 2.0 - 1.0;
                global.entities.push(new Shot(that.x, that.y, a, obj.eid));
            }
            return true;
        });
    }
};

Player.prototype.render = function() {
    global.line(this.x, this.y, this.x  , this.y-10, this.color);
    global.line(this.x, this.y, this.x+5, this.y+ 5, this.color);
    global.line(this.x, this.y, this.x-5, this.y+ 5, this.color);
};


/////////////////////////////////////////////////////////////////////////////
function Enemy(x, y) {
    this.className = "Enemy";
    this.x = x;
    this.y = y;
    this.vx = 0;
    this.vy = 0;
    this.color = '#8888ff';
}

Enemy.prototype.update = function() {
    moveBounce(this);
    this.vy += 0.1;
    if(this.y == global.height) {
        this.vx = random(-100, 100) / global.frameRate;
        this.vy = random(-240, -180) / global.frameRate;
    }
};

Enemy.prototype.render = function() {
    global.line(this.x, this.y, this.x-15, this.y-10, this.color);
    global.line(this.x, this.y, this.x+15, this.y-10, this.color);
    global.line(this.x, this.y, this.x- 5, this.y+10, this.color);
    global.line(this.x, this.y, this.x+ 5, this.y+10, this.color);
};


/////////////////////////////////////////////////////////////////////////////
function Shot(x, y, a, enemyId) {
    var hitTimeInSeconds = 0.75;
    this.className  = "Shot";
    this.p0         = { x: x, y: y};
    this.p1         = { x: x + a*global.width, y: y+150 };
    this.enemyId    = enemyId;
    this.color      = '#88ff88';
    this.a          = a;
    this.t          = 0.0;
    this.dt         = 1.0 / (global.frameRate * hitTimeInSeconds);
    this.points     = [];
};

Shot.prototype.update = function() {
    if(this.t > 1.0) {
        return true;
    }

    var e = global.entities.get(this.enemyId);
    var p2 = {
          x: e.x - this.a * global.width * 0.25
        , y: e.y - 90
    };
    var p3 = {
          x:e.x
        , y:e.y
    };

    if(this.t == 0) {
        this.points.push(cubicBezier(this.p0, this.p1, p2, p3, this.t));
    }

    this.t += this.dt;
    this.points.push(cubicBezier(this.p0, this.p1, p2, p3, this.t));
};

Shot.prototype.render = function() {
    for(var i = 1, n = this.points.length; i < n; ++i) {
        var t0 = this.points[i-1];
        var t1 = this.points[i];
        global.line(t0.x, t0.y, t1.x, t1.y, this.color);
    }
};


global = new Global(document.getElementById('myCanvas'));
global.setDebugLoggerElement(document.getElementById('debugLogArea'));
window.addEventListener('blur', function(e){ global.activate(false); });
window.addEventListener('focus', function(e){ global.activate(true); });
global.init(window);
global.activate(true);
