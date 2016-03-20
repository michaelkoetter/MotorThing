'use strict';
import gulp from 'gulp';
import gutil from 'gulp-util';
import concat from 'gulp-concat';
import sourcemaps from 'gulp-sourcemaps';
import uglify from 'gulp-uglify';
import cssnano from 'gulp-cssnano';
import gzip from 'gulp-gzip';
import webserver from 'gulp-webserver';
import RevAll from 'gulp-rev-all';
import hashsum from 'gulp-hashsum';
import flatten from 'gulp-flatten';
import filter from 'gulp-filter';
import eslint from 'gulp-eslint';
import browserify from 'browserify'
import source from 'vinyl-source-stream';
import buffer from 'vinyl-buffer';

import del from 'del';
import Path from 'path';
import mainBowerFiles from 'main-bower-files';

const src = {
  css: mainBowerFiles({ includeSelf: true, filter: "**/*.css" }),
  html: mainBowerFiles({ includeSelf: true, filter: ["**/*.html", "!**/tags/*.html"] }),
  woff: mainBowerFiles({ includeSelf: true, filter: "**/*.woff" })
}

gulp.task('default', ['compress']);

gulp.task('clean', () => {
  return del(['build', 'data/web']);
});

gulp.task('webserver', ['rev-all'], () => {
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

gulp.task('compress', ['rev-all'], () => {
  // don't compress WOFF files (until the webserver recognizes them)
  var f = filter(['**/*', '!**/*.woff'], {restore: true});
  var fSourceMaps = filter(['**/*', '!**/*.map']);

  return gulp.src('build/web-rev/**')
    .pipe(f)
    .pipe(fSourceMaps)
    .pipe(gzip())
    .pipe(f.restore)
    .pipe(gulp.dest('data/web'))
    .pipe(hashsum({hash: 'md5', dest: 'data/web'}));
});

gulp.task('rev-all', ['build'], () => {
  var revAll = new RevAll({
    transformFilename: function(file, hash) {
      if (file.path.endsWith('index.html'))
        return 'index.htm';

      return hash.substr(0,16) + Path.extname(file.path);
    },
    transformPath: function(rev, source, path) {
      return Path.basename(rev);
    }
  });

  return gulp.src('build/web/**')
    .pipe(revAll.revision())
    .pipe(flatten())
    .pipe(gulp.dest('build/web-rev'));

})

gulp.task('build', ['js-all', 'css-all', 'html-all', 'fonts-all']);

// Concatenate & minify all JavaScript source files
gulp.task('js-all', ['clean', 'js-lint'], () => {
  let b = browserify({
    entries: 'src/web/js/app.js',
    debug: true
  });

  return b.bundle()
    .pipe(source('app.js'))
    .pipe(buffer())
    .pipe(sourcemaps.init({ loadMaps: true }))
    .pipe(uglify())
    .pipe(sourcemaps.write('./'))
    .pipe(gulp.dest('build/web/js'));
});

gulp.task('js-lint', () => {
  return gulp.src(['src/web/**/*.js', 'src/web/tags/*.html'])
    .pipe(eslint())
    .pipe(eslint.format())
    .pipe(eslint.failOnError());
});

// Concatenate & minify all CSS source files
gulp.task('css-all', ['clean'], () => {
  return gulp.src(src.css)
    .pipe(sourcemaps.init())
    .pipe(concat('app.css'))
    .pipe(cssnano())
    .pipe(sourcemaps.write('./'))
    .pipe(gulp.dest('build/web/css'));
});

// Copy all HTML source files
gulp.task('html-all', ['clean'], () => {
  return gulp.src(src.html, {base: 'src/web'})
    .pipe(gulp.dest('build/web'));
});

// Copy fonts
gulp.task('fonts-all', ['clean'], () => {
  return gulp.src(src.woff)
    .pipe(gulp.dest('build/web/fonts'));
})
