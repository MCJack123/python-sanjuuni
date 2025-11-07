# python-sanjuuni
Converts images and videos into a format suitable for ComputerCraft, based on sanjuuni.

## Usage
Import this library using `import`:

```py
import sanjuuni
```

You may call `initOpenCL` before doing any conversion to use GPU acceleration via OpenCL if available. Note that OpenCL is not currently built by default, but power users may recompile the module to link in OpenCL.

Image conversion generally has four (or five including source decoding) steps:
1. Load the raw pixel data into an `RGBImage` instance.
2. Generate an optimized palette using a `reducePalette` function.
3. Convert the image to an indexed image using `thresholdImage` or a `ditherImage` function.
4. Generate the final output using one of the `make*` functions.

Alternatively, using CIELab color space for more optimal color accuracy:
1. Load the raw pixel data into an `RGBImage` instance.
2. Convert the image into Lab space with `makeLabImage`.
3. Generate an optimized palette using a `reducePalette` function.
4. Convert the image to an indexed image using `thresholdImage` or a `ditherImage` function.
5. Convert the palette back into RGB space with `convertLabPalette`.
6. Generate the final output using one of the `make*` functions.

For example, to generate a Lua script using Lab color, k-means quantization and Floyd-Steinberg dithering, operating on a BGRA buffer of pixels:

```py
img = sanjuuni.makeRGBImage(pixels, width, height, 'bgra')
lab = sanjuuni.makeLabImage(img)
labPalette = sanjuuni.reducePalette_kMeans(lab)
idxImg = sanjuuni.ditherImage_floydSteinberg(lab, labPalette)
palette = sanjuuni.convertLabPalette(labPalette)
luaFile = sanjuuni.makeLuaFile(idxImg, palette)
```

Note that this module does not have any built-in image decoding capabilities; use other modules to decode files if necessary.

See the typing file `sanjuuni/__init__.pyi` for complete documentation on the available functions.

## License
python-sanjuuni is licensed under the GPLv2 license (or later at your choice).
