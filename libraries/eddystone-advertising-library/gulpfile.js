// jshint node: true

// Ignore vars in require statements
// jshint ignore:start
var gulp = require('gulp');
var concat = require('gulp-concat');
var mocha = require('gulp-mocha');
var jshint = require('gulp-jshint');
var david = require('gulp-david');

var del = require('del');
// jshint ignore:end


gulp.task('default', () => {
});

gulp.task('test', [
  'test:dependencies',
  'test:style',
  'test:chrome-os',
  'clean']
);

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
  return gulp.src(['test/setup/chrome-os.js',
                   'test/eddystone-tests.js',
                   'eddystone-advertising.js'])
             .pipe(concat('chrome-os-tests.js'))
             .pipe(gulp.dest('./temp/'))
             .pipe(mocha({reporter: 'spec'}));
});

gulp.task('clean', ['test:chrome-os'], cb => {
  del('temp/', cb);
});
