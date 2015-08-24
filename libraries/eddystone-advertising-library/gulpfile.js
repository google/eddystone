// jshint node: true

// Ignore vars in require statements
// jshint ignore:start
var gulp = require('gulp');
var concat = require('gulp-concat');
var mocha = require('gulp-mocha');
var jshint = require('gulp-jshint');
var david = require('gulp-david');
var gulpJsdoc2md = require('gulp-jsdoc-to-markdown');
var rename = require('gulp-rename');
var diff = require('gulp-diff');

var del = require('del');
// jshint ignore:end

gulp.task('default', () => {
});

gulp.task('docs', () => {
  // TODO: Figure out how templates work so that we can include
  // the documentation directly in the README.md.
  return gulp.src('eddystone-advertising.js')
             .pipe(gulpJsdoc2md())
             .pipe(rename(path => path.extname = '.md'))
             .pipe(gulp.dest(''));
});

gulp.task('test', [
  'test:docs',
  'test:dependencies',
  'test:style',
  'test:chrome-os',
  'clean']
);

gulp.task('test:docs', () => {
  return gulp.src('eddystone-advertising.js')
             .pipe(gulpJsdoc2md())
             .pipe(rename(path => path.extname = '.md'))
             .pipe(diff())
             .pipe(diff.reporter({fail: true}));
});

gulp.task('test:dependencies', () => {
  return gulp.src('package.json')
             .pipe(david({error404: true}))
             .pipe(david.reporter);
});

gulp.task('test:style', () => {
  return gulp.src(['eddystone-advertising.js', 'gulpfile.js','test/*.js',
                   'test/**/*.js'])
             .pipe(jshint())
             .pipe(jshint.reporter('jshint-stylish'))
             .pipe(jshint.reporter('fail'));
});

// TODO: Maybe use browserify instead of concatenating to test.
gulp.task('test:chrome-os', () => {
  return gulp.src([
    // Setup
    'test/setup/chrome-os.js',
    // Library
    'eddystone-advertising.js',
    // Tests
    'test/eddystone-tests.js',
    'test/eddystone-url-tests.js',
    'test/eddystone-chrome-os-tests.js'
  ]).pipe(concat('chrome-os-tests.js'))
    .pipe(gulp.dest('./temp/'))
    .pipe(mocha({reporter: 'spec'}));
});

gulp.task('clean', ['test:chrome-os'], cb => {
  del('temp/', cb);
});
