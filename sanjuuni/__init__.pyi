from typing import TypedDict, Literal, overload

class ColorDict(TypedDict):
    r: int
    g: int
    b: int

Color = tuple[int, int, int] | ColorDict
""" A color, usually in RGB, but may also be holding Lab coordinates. """
Palette = list[Color]
""" A palette of colors. """
LabPalette = Palette
""" A palette of Lab colors. """

class RGBImage:
    """
    Holds an RGB image.
    Wrapper around sanjuuni `Mat`.
    """
    width: int
    height: int
    def at(x: int, y: int) -> Color: ...

class LabImage(RGBImage): ...

class IndexedImage:
    """
    Holds an 8-bit indexed image.
    Wrapper around sanjuuni `Mat1b`.
    """
    width: int
    height: int
    def at(x: int, y: int) -> int: ...

def initOpenCL(device: int | Literal["best_flops"] | Literal["best_memory"] = "best_flops") -> bool:
    """
    Initializes OpenCL support if available.
    
    :param device: The device to use; specify "best_flops" for device with most computation power, or "best_memory" for device with most memory
    :returns: Whether OpenCL initialization succeeded (if it failed, sanjuuni will still work in CPU-only mode)
    """
    ...

@overload
def makeRGBImage(image: list[list[Color]]) -> RGBImage:
    """
    Creates an image from a 2D color array.
    
    :param image: The image source
    :returns: The new RGB image
    """
    ...

@overload
def makeRGBImage(image: bytes, width: int, height: int, format: Literal["rgb"] | Literal["rgba"] | Literal["bgr"] | Literal["bgra"] | Literal["argb"] | Literal["abgr"]) -> RGBImage:
    """
    Creates an image from a byte buffer.
    
    :param image: The image source
    :param width: The width of the image
    :param height: The height of the image
    :param format: The format the data is in, i.e. the byte order
    :returns: The new RGB image
    """
    ...

@overload
def makeRGBImage(image: bytearray, width: int, height: int, format: Literal["rgb"] | Literal["rgba"] | Literal["bgr"] | Literal["bgra"] | Literal["argb"] | Literal["abgr"]) -> RGBImage:
    """
    Creates an image from a byte buffer.
    
    :param image: The image source
    :param width: The width of the image
    :param height: The height of the image
    :param format: The format the data is in, i.e. the byte order
    :returns: The new RGB image
    """
    ...

@overload
def makeRGBImage(image: list[int], width: int, height: int, format: Literal["rgba"] | Literal["bgra"] | Literal["argb"] | Literal["abgr"]) -> RGBImage:
    """
    Creates an image from a 32-bit integer buffer.
    
    :param image: The image source
    :param width: The width of the image
    :param height: The height of the image
    :param format: The format the data is in, i.e. the byte order
    :returns: The new RGB image
    """
    ...

def makeLabImage(image: RGBImage) -> LabImage:
    """
    Converts an sRGB image into CIELAB color space.

    :param image: The image to convert
    :returns: A new image with all pixels in Lab color space
    """
    ...

def convertLabPalette(palette: LabPalette) -> Palette:
    """
    Converts a list of Lab colors into sRGB colors.

    :param palette: The colors to convert
    :returns: A new list with all colors converted to RGB
    """
    ...

@overload
def reducePalette_medianCut(image: RGBImage, numColors: int = 16) -> Palette:
    """
    Generates an optimized palette for an image using the median cut algorithm.
    
    :param image: The image to generate a palette for
    :param numColors: The number of colors to get (must be a power of 2)
    :returns: An optimized palette for the image
    """
    ...

@overload
def reducePalette_medianCut(image: LabImage, numColors: int = 16) -> LabPalette:
    """
    Generates an optimized palette for an image using the median cut algorithm.
    
    :param image: The image to generate a palette for
    :param numColors: The number of colors to get (must be a power of 2)
    :returns: An optimized palette for the image
    """
    ...

@overload
def reducePalette_kMeans(image: RGBImage, numColors: int = 16) -> Palette:
    """
    Generates an optimized palette for an image using the k-means algorithm.
    
    :param image: The image to generate a palette for
    :param numColors: The number of colors to get
    :returns: An optimized palette for the image
    """
    ...

@overload
def reducePalette_kMeans(image: LabImage, numColors: int = 16) -> LabPalette:
    """
    Generates an optimized palette for an image using the k-means algorithm.
    
    :param image: The image to generate a palette for
    :param numColors: The number of colors to get
    :returns: An optimized palette for the image
    """
    ...

@overload
def reducePalette_octree(image: RGBImage, numColors: int = 16) -> Palette:
    """
    Generates an optimized palette for an image using octrees.
    
    :param image: The image to generate a palette for
    :param numColors: The number of colors to get
    :returns: An optimized palette for the image
    """
    ...

@overload
def reducePalette_octree(image: LabImage, numColors: int = 16) -> LabPalette:
    """
    Generates an optimized palette for an image using octrees.
    
    :param image: The image to generate a palette for
    :param numColors: The number of colors to get
    :returns: An optimized palette for the image
    """
    ...

@overload
def thresholdImage(image: RGBImage, palette: Palette) -> IndexedImage:
    """
    Reduces the colors in an image using the specified palette through thresholding.
    
    :param image: The image to reduce
    :param palette: The palette to use
    :returns: A reduced-color version of the image using the palette
    """
    ...

@overload
def thresholdImage(image: LabImage, palette: LabPalette) -> IndexedImage:
    """
    Reduces the colors in an image using the specified palette through thresholding.
    
    :param image: The image to reduce
    :param palette: The palette to use
    :returns: A reduced-color version of the image using the palette
    """
    ...

@overload
def ditherImage_ordered(image: RGBImage, palette: Palette) -> IndexedImage:
    """
    Reduces the colors in an image using the specified palette through ordered
    dithering.
    
    :param image: The image to reduce
    :param palette: The palette to use
    :returns: A reduced-color version of the image using the palette
    """
    ...

@overload
def ditherImage_ordered(image: LabImage, palette: LabPalette) -> IndexedImage:
    """
    Reduces the colors in an image using the specified palette through ordered
    dithering.
    
    :param image: The image to reduce
    :param palette: The palette to use
    :returns: A reduced-color version of the image using the palette
    """
    ...

@overload
def ditherImage_floydSteinberg(image: RGBImage, palette: Palette) -> IndexedImage:
    """
    Reduces the colors in an image using the specified palette through Floyd-
    Steinberg dithering.
    
    :param image: The image to reduce
    :param palette: The palette to use
    :returns: A reduced-color version of the image using the palette
    """
    ...

@overload
def ditherImage_floydSteinberg(image: LabImage, palette: LabPalette) -> IndexedImage:
    """
    Reduces the colors in an image using the specified palette through Floyd-
    Steinberg dithering.
    
    :param image: The image to reduce
    :param palette: The palette to use
    :returns: A reduced-color version of the image using the palette
    """
    ...

def makeTable(image: IndexedImage, palette: Palette, compact: bool = False, embedPalette: bool = False, binary: bool = False) -> str:
    """
    Generates a blit table from the specified CC image.
    
    :param input: The image to convert
    :param palette: The palette for the image
    :param compact: Whether to make the output as compact as possible
    :param embedPalette: Whether to embed the palette as a `palette` key (for BIMG)
    :returns: The generated blit image source for the image data
    """
    ...
def makeNFP(image: IndexedImage, palette: Palette) -> str:
    """
    Generates an NFP image from the specified CC image. This changes proportions!
    :param input: The image to convert
    :param palette: The palette for the image
    :returns: The generated NFP for the image data
    """
    ...
def makeLuaFile(image: IndexedImage, palette: Palette) -> str:
    """
    Generates a Lua display script from the specified CC image. This file can be
    run directly to show the image on-screen.
    
    :param input: The image to convert
    :param palette: The palette for the image
    :returns: The generated  for the image data
    """
    ...
def makeRawImage(image: IndexedImage, palette: Palette) -> str:
    """
    Generates a raw mode frame from the specified CC image.
    
    :param input: The image to convert
    :param palette: The palette for the image
    :returns: The generated  for the image data
    """
    ...
def make32vid(image: IndexedImage, palette: Palette) -> bytes:
    """
    Generates an uncompressed 32vid frame from the specified CC image.
    
    :param input: The image to convert
    :param palette: The palette for the image
    :returns: The generated  for the image data
    """
    ...
def make32vid_cmp(image: IndexedImage, palette: Palette) -> bytes:
    """
    Generates a 32vid frame from the specified CC image using the custom compression scheme.
    
    :param input: The image to convert
    :param palette: The palette for the image
    :returns: The generated  for the image data
    """
    ...
def make32vid_ans(image: IndexedImage, palette: Palette) -> bytes:
    """
    Generates a 32vid frame from the specified CC image using the custom ANS compression scheme.
    
    :param input: The image to convert
    :param palette: The palette for the image
    :returns: The generated  for the image data
    """
    ...
