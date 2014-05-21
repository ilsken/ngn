#!/usr/bin/env node
var exec = require('child_process').exec;
var spawn = require('child_process').spawn;
var path = require('path');
var os  = require('os');
var fmt = require('util').format;
var exists = require('fs').existsSync;
var env = process.env;

var cwd = process.cwd();
var root_dir = __dirname;
var output_dir = path.join(root_dir, 'out');
var compiler = env.CC || 'cc';
var gyp = env.GYP || 'gyp';
var args = process.argv.slice(2);

var files = ['ngn.gyp', 'common.gypi', 'options.gypi'].filter(exists);

function set(name, value) {
  args.push(fmt('--%s=%s', name, value));
}

function define(name, value) {
  args.push(fmt('-D%s=%s', name, value))
}

function include(path) {
  args.push('-I', path);
}

function compiler_version(callback) {
  exec(compiler + ' --version', function (error, out) {
    var ret = {clang: false, major: 0, minor: 0};
    if (error) return callback(error);
    ret.clang = out.indexOf('clang') > -1;
    exec(compiler + ' -dumpversion', function (error, out) {
      if (error) return callback(error, ret);
      var split = out.split('.');
      ret.major = parseInt(split[0]);
      ret.minor = parseInt(split[1]);
      callback(null, ret);
    })
  })
}
/*
function detect_headers(headers, done) {
  var src = headers.map(function(header) {
    return ['#include <', header, '>'].join('');
  }).join('\n');
  spawn(compiler, [''])
}*/

if (os.platform() == 'win32') {
  env.GYP_MSVS_VERSION = '2010'
  files = files.map(relative);
} else {
  files = files.map(absolute);
}

args.push(files.shift());
files.forEach(include);

set('depth', root_dir);
if (os.platform() != 'win32') {
  if (!has('-f'))
    args.push('-f', 'make');
  if (!has('eclipse') && !has('ninja')) {
    args.push(fmt('-Goutput_dir="%s"', output_dir));
    set('generator-output', output_dir);
  }
  compiler_version(function (error, compiler) {
    if (error) throw 'Failed to execute compiler, make sure it\'s in your path';
    define('gcc_version', (10 * compiler.major + compiler.minor));
    define('clang', Number(compiler.clang));
    run();
  })
} else {
  run();
}

function run() {
  setDefault('host_arch', os.arch());
  setDefault('target_arch', os.arch());
  setDefault('library', 'static_library');
  setDefault('component', 'static_library');
  var proc = spawn('gyp', args);
  proc.stdout.pipe(process.stdout);
  proc.stderr.pipe(process.stderr);
}


function relative(file) { return path.relative(cwd, root_dir) }
function absolute(file) { return path.join(root_dir, file); }
function has(str) {
  return args.indexOf(str) > -1;
}

function setDefault(name, value) {
  if (!args.some(startsWith(fmt('-D%s=', name))))
    define(name, value);
}

function startsWith(prefix) {
  return function (str) {
    return str.indexOf(prefix) === 0;
  }
}


