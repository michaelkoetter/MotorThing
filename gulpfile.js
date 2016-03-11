var gulp        = require('gulp');
var gutil       = require('gulp-util');
var clean       = require('gulp-clean');
var rename      = require('gulp-rename');
var concat      = require('gulp-concat');
var uglify      = require('gulp-uglify');
var cssnano 	  = require('gulp-cssnano');
var htmlmin     = require('gulp-htmlmin');
var gzip        = require('gulp-gzip');
var hashsum     = require("gulp-hashsum");
var mainBowerFiles  = require('main-bower-files');
var fnv         = require('fnv-plus');
var webserver = require('gulp-webserver');

var src = {
  js: mainBowerFiles({includeSelf: true, filter: "**/*.js", debugging: true}),
  css: mainBowerFiles({includeSelf: true, filter: "**/*.css"}),
  html: mainBowerFiles({includeSelf: true, filter: ["**/*.html", "**/*.tag"]}),
  woff: mainBowerFiles({includeSelf: true, filter: "**/*.woff"}),
}

gulp.task('default', ['compress-data']);
gulp.task('build', ['js-all', 'css-all', 'html-all', 'fonts-all']);

gulp.task('clean', function() {
  return gulp.src(['build/web', 'data/web'], {read: false})
    .pipe(clean());
});

gulp.task('webserver', ['build'], function() {
  return gulp.src('build/web')
    .pipe(webserver({
      livereload: true,
      open: true
    }));
});

// This prepares the static files for the embedded web server
// - Filenames are shortened using FNV1a hash
//   This way we don't need to obey the 32 char limit of SPIFFS filenames
// - The resulting files are compressed using gzip
//   We need to save as much space as possible.
// - MD5 checksums are created
//   These could be used by the webserver for ETag headers (not implemented yet)
gulp.task('compress-data', ['build'], function() {
  return gulp.src(['build/web/*.*', 'build/web/**/*.*'])
    .pipe(rename(function(path) {
      if (path.extname) {
        // build short filename
        // the path relative to the FSRequestHandler's uri will be hashed
        var _path =
          (path.dirname != '.' ? path.dirname.replace('\\', '/') + '/' : '')
          + path.basename
          + path.extname;

        gutil.log('path: ', _path);
        fnv.seed(0);
        path.basename = fnv.hash(_path, 32).hex();
        gutil.log('short path: ', path.basename);
        path.dirname = '.';
        path.extname = '';
      }
    }))
    .pipe(gzip())
    .pipe(gulp.dest('data/web'))
    .pipe(hashsum({hash: 'md5', dest: 'data/web'}));
});


// Concatenate & minify all JavaScript source files
gulp.task('js-all', function() {
  return gulp.src(src.js)
    .pipe(concat('motor-thing.js'))
    .pipe(rename({suffix: '.min'}))
    .pipe(uglify())
    .pipe(gulp.dest('build/web/js'));
});

// Concatenate & minify all CSS source files
gulp.task('css-all', function() {
  return gulp.src(src.css)
    .pipe(concat('motor-thing.css'))
    .pipe(rename({suffix: '.min'}))
    .pipe(cssnano())
    .pipe(gulp.dest('build/web/css'));
});

// Concatenate & minify all HTML source files
gulp.task('html-all', function() {
  return gulp.src(src.html, {base: 'src/web'})
    .pipe(htmlmin({
      // this seems to be all we can do without breaking Riot tag syntax
      removeComments: true,
      minifyCSS: true,
      minifyURLs: true,
    }))
    .pipe(gulp.dest('build/web'));
});

// Copy fonts
gulp.task('fonts-all', function() {
  return gulp.src(src.woff)
    .pipe(gulp.dest('build/web/fonts'));
})
