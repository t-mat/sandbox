/////////////////////////////////////////////////////////////////////////////
function Entities() {
    var idShift         = 10;
    var indexMask       = (1 << idShift) - 1;
    var maxEntityCount  = 1024;

    var idCounter       = 1;
    var entities        = new Array(maxEntityCount);
    var that            = this;

    function init() {
        that.clear();
    }

    this.clear = function() {
        entities = [];
        for(var i = 0; i < maxEntityCount; ++i) {
            entities.push(null);
        }
    };

    this.getIndexFromId = function(id) {
        return id & indexMask;
    };

    this.createId = function(index) {
        return index | (idCounter++ << idShift);
    };

    this.get = function(id) {
        var idx = this.getIndexFromId(id);
        var e = entities[idx];
        if(e) {
            return e;
        }
        return null;
    };

    this.push = function(obj) {
        for(var i = 0, n = entities.length; i < n; ++i) {
            if(! entities[i]) {
                var id = this.createId(i);
                entities[i] = obj;
                entities[i].eid = id;
                return id;
            }
        }
        return null;
    };

    this.remove = function(id) {
        var idx = this.getIndexFromId(id);
        var e = entities[idx];
        if(e && e.eid == id) {
            entities[idx] = null;
        }
    };

    this.update = function() {
        var deleteIds = [];
        for(var i = 0; i < entities.length; ++i) {
            var e = entities[i];
            if(e) {
                var b = e.update();
                if(b === true) {
                    deleteIds.push(e.eid);
                }
            }
        }

        for(var d = deleteIds.length-1; d >= 0; --d) {
            this.remove(deleteIds[d]);
        }
    };

    this.render = function() {
        for(var i = 0; i < entities.length; ++i) {
            var e = entities[i];
            if(e) {
                e.render();
            }
        }
    };

    this.every = function(callbackFunc) {
        for(var i = 0; i < entities.length; ++i) {
            var e = entities[i];
            if(e) {
                if(! callbackFunc(e)) {
                    break;
                }
            }
        }
    };

    init();
}


/////////////////////////////////////////////////////////////////////////////
function DebugLogger(element) {
    var el = element;

    if(!el) {
        this.log = function() {};
        this.vlogf = function() {};
        this.logf = function() {};
        return;
    }

    this.log = function(msg) {
        try {
            el.innerHTML += msg;
        } catch(e) {}
    };

    this.vlogf = function(args) {
        // http://stackoverflow.com/questions/610406/javascript-equivalent-to-printf-string-format
        //  vlogf("{1} - {0} - {2}", 'A', 'B', 'C');
        //  --> "B - A - C"
        this.log(
              args[0].replace(/{(\d+)}/g
            , function(match, number) {
                return typeof args[number] != 'undefined' ? args[(number<<0)+1] : match;
            })
        );
    };

    this.logf = function() {
        this.vlogf(arguments);
    };
}


/////////////////////////////////////////////////////////////////////////////
function Keyboard(element) {
    this.KEY = {
          NUL:0, BACKSPACE:8, TAB:9, ENTER:13, SHIFT:16, CTRL:17, ALT:18
        , ESCAPE:27, SPACE:32, PAGEUP:33, PAGEDOWN:34, END:35, HOME:36
        , LEFT:37, UP:38, RIGHT:39, DOWN:40, INSERT:45, DELETE:46
        , _0:48, _1:49, _2:50, _3:51, _4:52, _5:53, _6:54, _7:55, _8:56, _9:57
        , A:65, B:66, C:67, D:68, E:69, F:70, G:71, H:72, I:73, J:74, K:75
        , L:76, M:77, N:78, O:79, P:80, Q:81, R:82, S:83, T:84, U:85, V:86
        , W:87, X:88, Y:89, Z:90
        , F1:112, F2:113, F3:114, F4:115, F5:116, F6:117, F7:118, F8:119
        , F9:120, F10:121, F11:122, F12:123
    };
    this.on     = new Array(256);
    this.down   = new Array(256);   // down trigger
    this.up     = new Array(256);   // up trigger

    var that    = this;
    var buf     = new Array(256);

    function init(element) {
        that.clear();

        element.addEventListener('keydown', function(e) {
            buf[e.keyCode] = true;
            return false;
        });

        element.addEventListener('keyup', function(e) {
            buf[e.keyCode] = false;
            return false;
        });
    }

    this.clear = function() {
        for(var i = 0; i < this.on.length; ++i) {
            this.on[i] = false;
            this.down[i] = false;
            this.up[i] = false;
            buf[i] = false;
        }
    };

    this.update = function() {
        for(var i = 0; i < this.on.length; ++i) {
            if(this.on[i] != buf[i]) {
                if(buf[i]) {
                    // up -> down
                    this.on[i] = true;
                    this.down[i] = true;
                    this.up[i] = false;
                } else {
                    // down -> up
                    this.on[i] = false;
                    this.down[i] = false;
                    this.up[i] = true;
                }
            } else {
            //  this.on[i] = buf[i];
                this.down[i] = false;
                this.up[i] = false;
            }
        }
    }

    init(element);
}
