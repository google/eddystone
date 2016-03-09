// jshint node: true
'use strict';

import gulp from 'gulp';
import concat from 'gulp-concat';
import mocha from 'gulp-mocha';
import jshint from 'gulp-jshint';
import david from 'gulp-david';
import gulpJsdoc2md from 'gulp-jsdoc-to-markdown';
import fs from 'fs';
import rename from 'gulp-rename';
import diff from 'gulp-diff';

import browserify from 'browserify';
import source from 'vinyl-source-stream';
import buffer from 'vinyl-buffer';
import uglify from 'gulp-uglify';

import del from 'del';

gulp.task('default', () => {
});

gulp.task('docs', () => {
  return getJsDocPipe()
    .pipe(gulp.dest(''));
});

gulp.task('test', [
  'test:docs',
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
             .pipe(david({error404: true}));
});

gulp.task('test:style', () => {
  return gulp.src(['gulpfile.babel.js',
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
