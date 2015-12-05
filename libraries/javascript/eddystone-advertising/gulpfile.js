// jshint node: true

// Ignore vars in require statements
// jshint ignore:start
var gulp = require('gulp');
var concat = require('gulp-concat');
var mocha = require('gulp-mocha');
var jshint = require('gulp-jshint');
var david = require('gulp-david');
var gulpJsdoc2md = require('gulp-jsdoc-to-markdown');
var fs = require('fs');
var rename = require('gulp-rename');
var diff = require('gulp-diff');

var browserify = require('browserify');
var source = require('vinyl-source-stream');
var buffer = require('vinyl-buffer');
var uglify = require('gulp-uglify');

var del = require('del');
// jshint ignore:end

gulp.task('default', () => {
});

gulp.task('docs', () => {
  return getJsDocPipe()
    .pipe(gulp.dest(''));
});

gulp.task('test', [
// TODO(g-ortuno): Uncomment once jsdoc2md is fixed:
// https://github.com/jsdoc2md/jsdoc-to-markdown/issues/29
//  'test:docs',
  'test:dependencies',
  'test:browserify',
  'test:style',
  'test:unit']
);

gulp.task('browserify', () => {

  return getBrowserifyPipe()
    .pipe(gulp.dest(''));
});

gulp.task('test:browserify', () => {
  'use strict';

  return getBrowserifyPipe()
    .pipe(diff())
    .pipe(diff.reporter({fail: true}));
});

gulp.task('test:docs', () => {
  return getJsDocPipe()
    .pipe(diff('.'))
    .pipe(diff.reporter({fail: true}));
});

gulp.task('test:dependencies', () => {
  return gulp.src('package.json')
             .pipe(david({error404: true}))
             .pipe(david.reporter);
});

gulp.task('test:style', () => {
  return gulp.src(['gulpfile.js',
                   'lib/*.js',
                   'test/unit/*.js'])
             .pipe(jshint())
             .pipe(jshint.reporter('jshint-stylish'))
             .pipe(jshint.reporter('fail'));
});

gulp.task('test:unit', () => {
  return gulp.src('test/unit/*.js', {read: false})
             .pipe(mocha({reporter: 'spec'}));
});

gulp.task('clean', ['test:chrome-os'], cb => {
  del('temp/', cb);
});

function getJsDocPipe() {
  return gulp.src('lib/*.js')
             .pipe(concat('README.md'))
             .pipe(gulpJsdoc2md({
               template: fs.readFileSync('jsdoc2md/README.hbs', 'utf-8')
             }));
}

function getBrowserifyPipe() {
  'use strict';

  return browserify('./lib/exports.js', {
    // Ignore external modules, in this case TextEncoder.
    bundleExternal: false
  }).bundle()
    .pipe(source('eddystone-advertising.js'))
    .pipe(buffer());
    // Uncomment once uglify supports arrow functions:
    // https://github.com/mishoo/UglifyJS2/issues/448
    /* .pipe(uglify()); */
}
