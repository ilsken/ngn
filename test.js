var em = require('events').EventEmitter;
var ev = new em();
ev.on('foo', function (){
  console.log(1);
  ev.on('foo', function () {
    console.log(2);
  });
});

ev.emit('foo');

function first() {
  console.log('first');
  ev.removeListener('bar',second);
}
function second(){
  console.log('second');
}

ev.on('bar', first);
ev.on('bar', second);
ev.emit('bar');
