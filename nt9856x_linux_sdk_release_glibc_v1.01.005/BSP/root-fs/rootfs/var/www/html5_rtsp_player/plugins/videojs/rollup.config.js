import babel from 'rollup-plugin-babel';
import alias from 'rollup-plugin-alias';
import uglify from 'rollup-plugin-uglify';

const path = require('path');

export default {
    entry: path.join(__dirname, 'src/videojs.streamedian.js'),
    targets: [
        {
            dest: path.join(__dirname, 'example/streamedian.videojs.min.js'),
            format: 'iife',
            name: 'streamedian.videojs'
        }
    ],
    sourceMap: true,
    plugins: [
        babel({
            // exclude: 'node_modules/**',
        }),
        alias({
            streamedian: path.join(__dirname,'../../src')
        })
    ]
}