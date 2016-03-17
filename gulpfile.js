var gulp        = require('gulp');
var gutil       = require('gulp-util');
var clean       = require('gulp-clean');
var concat      = require('gulp-concat');
var uglify      = require('gulp-uglify');
var cssnano 	  = require('gulp-cssnano');
var htmlmin     = require('gulp-htmlmin');
var gzip        = require('gulp-gzip');
var hashsum     = require("gulp-hashsum");
var mainBowerFiles  = require('main-bower-files');
var webserver = require('gulp-webserver');
var RevAll      = require('gulp-rev-all');
var Path        = require('path');
var flatten     = require('gulp-flatten');
var del         = require('del');
var filter      = require('gulp-filter');

var src = {
  js: mainBowerFiles({includeSelf: true, filter: "**/*.js"}),
  css: mainBowerFiles({includeSelf: true, filter: "**/*.css"}),
  html: mainBowerFiles({includeSelf: true, filter: ["**/*.html", "!**/tags/**"]}),
  riotTags: mainBowerFiles({includeSelf: true, filter: "**/tags/**"}),
  woff: mainBowerFiles({includeSelf: true, filter: "**/*.woff"}),
  all: mainBowerFiles({includeSelf: true}),
}

gulp.task('default', ['compress']);

gulp.task('clean', function() {
  return del(['build', 'data/web']);
});

gulp.task('webserver', ['rev-all'], function() {
  gulp.watch('src/web/**', ['rev-all']);

  return gulp.src('build/web-rev')
    .pipe(webserver({
      livereload: true,
      //open: 'index.htm',
      middleware: [
        function(req, res, next) {
          //gutil.log(req, res);
          if (req.url == '/tmcl') {
            if (req.method == 'GET') {
              res.end(JSON.stringify({version: 'SIMULATION'}))
            } else if (req.method == 'POST') {
              res.end(JSON.stringify({status: 100}))
            }
          } else {
            next();
          }
        }
      ]
    }));
});

gulp.task('compress', ['rev-all'], function() {
  // don't compress WOFF files (until the webserver recognizes them)
  var f = filter(['**/*', '!**/*.woff'], {restore: true});

  return gulp.src('build/web-rev/**')
    .pipe(f)
    .pipe(gzip())
    .pipe(f.restore)
    .pipe(gulp.dest('data/web'))
    .pipe(hashsum({hash: 'md5', dest: 'data/web'}));
});

gulp.task('rev-all', ['build'], function() {
  var revAll = new RevAll({
    transformFilename: function(file, hash) {
      if (file.path.endsWith('index.html'))
        return 'index.htm';

      return hash.substr(0,16) + Path.extname(file.path);
    },
    transformPath: function(rev, source, path) {
      return Path.basename(rev);
    },
  });

  return gulp.src('build/web/**')
    .pipe(revAll.revision())
    .pipe(flatten())
    .pipe(gulp.dest('build/web-rev'));

})

gulp.task('build', ['js-all', 'css-all', 'html-all', 'riot-all', 'fonts-all']);

// Concatenate & minify all JavaScript source files
gulp.task('js-all', ['clean'], function() {
  return gulp.src(src.js, {base: 'src/web'})
    .pipe(concat('app.min.js'))
    .pipe(uglify())
    .pipe(gulp.dest('build/web/js'));
});

// Concatenate & minify all CSS source files
gulp.task('css-all', ['clean'], function() {
  return gulp.src(src.css)
    .pipe(concat('app.min.css'))
    .pipe(cssnano())
    .pipe(gulp.dest('build/web/css'));
});

// Concatenate & minify all HTML source files
gulp.task('html-all', ['clean'], function() {
  return gulp.src(src.html, {base: 'src/web'})
    .pipe(gulp.dest('build/web'));
});

gulp.task('riot-all', ['clean'], function() {
  return gulp.src(src.riotTags)
    .pipe(concat('tags.html'))
    .pipe(gulp.dest('build/web'));
})

// Copy fonts
gulp.task('fonts-all', ['clean'], function() {
  return gulp.src(src.woff)
    .pipe(gulp.dest('build/web/fonts'));
})
