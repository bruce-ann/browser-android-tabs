description(
'Tests to make sure that dynamic scope objects are correctly protected from GC.  To pass we need to not crash.'
);

(function() {
    try {
        // Immediate value for scope
        with(1) { gc(); a; }
    } catch(e) {
    }
})();

(function() {
    try {
        // Real object for scope
        var local;
        with (local = {}) {
            z=null;
            {}; {}; [1,2,3,4*{}]; // Clobber any temporaries the scope may exist in
            gc(); 
            b;
        }
    } catch (e) {
    }
})();

(function() {
    try {
        // Test catch blocks for the heck of it
        try {
            throw 1;
        } catch(e) {
            gc();
            b;
        }
    } catch (e) {
    }
})();
