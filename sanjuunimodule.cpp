#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <sanjuuni.hpp>

WorkQueue work; // TODO: is this going to break on certain interpreters? https://docs.python.org/3/extending/extending.html#writing-extensions-in-c
OpenCL::Device * device = NULL;

static int nulltraverse(PyObject *self, visitproc visit, void *arg) {return 0;}

struct PyMat {
    PyObject_HEAD
    unsigned width;
    unsigned height;
    Mat * ptr;
};

static Mat * GetRGBImage(PyObject *obj);
static PyObject *RGBImageAt(PyObject *self, PyObject *args) {
    int x, y;
    if (!PyArg_ParseTuple(args, "ii", &x, &y)) return NULL;
    Mat * img = GetRGBImage(self);
    if (img == NULL) return NULL;
    img->download();
    try {
        const uchar3& color = img->at(x, y);
        PyObject *retval = PyTuple_New(3);
        PyTuple_SetItem(retval, 2, PyLong_FromUnsignedLong(color.x));
        PyTuple_SetItem(retval, 1, PyLong_FromUnsignedLong(color.y));
        PyTuple_SetItem(retval, 0, PyLong_FromUnsignedLong(color.z));
        return retval;
    } catch (const std::out_of_range &e) {
        PyErr_SetString(PyExc_IndexError, e.what()); return NULL;
    }
}

static void RGBImage_finalize(PyObject *obj) {delete ((PyMat*)obj)->ptr;}

static PyMemberDef RGBImage_members[] = {
    {"width", Py_T_LONG, offsetof(PyMat, width), 0, "The width of the image."},
    {"height", Py_T_LONG, offsetof(PyMat, height), 0, "The height of the image."},
    {NULL, 0, 0, 0, NULL}
};

static PyMethodDef RGBImage_methods[] = {
    {"at", RGBImageAt, METH_VARARGS, "Get a pixel in the image."},
    {NULL, NULL, 0, NULL}
};

static PyTypeObject RGBImage = {
    {},
    "sanjuuni.RGBImage",
    sizeof(PyMat), 0,
    NULL, 0, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_IMMUTABLETYPE | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    "Buffer containing an RGB image to be converted.",
    nulltraverse,
    NULL,
    NULL,
    0,
    NULL, NULL,
    RGBImage_methods,
    RGBImage_members,
    NULL,
    NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    0,
    RGBImage_finalize,
};

static Mat * GetRGBImage(PyObject *obj) {
    if (!PyObject_TypeCheck(obj, &RGBImage)) {PyErr_SetString(PyExc_TypeError, "RGBImage expected"); return NULL;}
    return ((PyMat*)obj)->ptr;
}

static PyObject *NewRGBImage(Mat * img) {
    PyMat *mat = (PyMat*)PyType_GenericAlloc(&RGBImage, 1);
    if (mat != NULL) {
        mat->ptr = img;
        mat->width = mat->ptr->width;
        mat->height = mat->ptr->height;
    }
    return (PyObject*)mat;
}

struct PyMat1b {
    PyObject_HEAD
    unsigned width;
    unsigned height;
    Mat1b * ptr;
};

static Mat1b * GetIndexedImage(PyObject *obj);
static PyObject *IndexedImageAt(PyObject *self, PyObject *args) {
    int x, y;
    if (!PyArg_ParseTuple(args, "ii", &x, &y)) return NULL;
    Mat1b * img = GetIndexedImage(self);
    img->download();
    try {
        const uint8_t& color = img->at(x, y);
        return PyLong_FromUnsignedLong(color);
    } catch (const std::out_of_range &e) {
        PyErr_SetString(PyExc_IndexError, e.what()); return NULL;
    }
}

static void IndexedImage_finalize(PyObject *obj) {delete ((PyMat1b*)obj)->ptr;}

static PyMemberDef IndexedImage_members[] = {
    {"width", Py_T_ULONG, offsetof(PyMat1b, width), Py_READONLY, "The width of the image."},
    {"height", Py_T_ULONG, offsetof(PyMat1b, height), Py_READONLY, "The height of the image."},
    {NULL, 0, 0, 0, NULL}
};

static PyMethodDef IndexedImage_methods[] = {
    {"at", IndexedImageAt, METH_VARARGS, "Get a pixel in the image."},
    {NULL, NULL, 0, NULL}
};

static PyTypeObject IndexedImage = {
    {},
    "sanjuuni.IndexedImage",
    sizeof(PyMat1b), 0,
    NULL, 0, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_IMMUTABLETYPE | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    "Buffer containing an indexed image to be converted.",
    nulltraverse,
    NULL,
    NULL,
    0,
    NULL, NULL,
    IndexedImage_methods,
    IndexedImage_members,
    NULL,
    NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    0,
    IndexedImage_finalize,
};

static Mat1b * GetIndexedImage(PyObject *obj) {
    if (!PyObject_TypeCheck(obj, &IndexedImage)) {PyErr_SetString(PyExc_TypeError, "IndexedImage expected"); return NULL;}
    return ((PyMat1b*)obj)->ptr;
}

static PyObject *NewIndexedImage(Mat1b * img) {
    PyMat1b *mat = (PyMat1b*)PyType_GenericAlloc(&IndexedImage, 1);
    if (mat != NULL) {
        mat->ptr = img;
        mat->width = mat->ptr->width;
        mat->height = mat->ptr->height;
    }
    return (PyObject*)mat;
}

static std::vector<Vec3b> GetPalette(PyObject *obj) {
    if (!PyList_Check(obj)) {PyErr_SetString(PyExc_TypeError, "Palette expected"); return {};}
    std::vector<Vec3b> retval;
    for (int i = 0; i < PyList_Size(obj); i++) {
        Vec3b val;
        PyObject *v = PyList_GetItem(obj, i);
        if (!PyTuple_Check(v)) {PyErr_SetString(PyExc_TypeError, "Palette expected"); return {};}
        PyObject *rv = PyTuple_GetItem(v, 2);
        if (!PyLong_Check(rv)) {PyErr_SetString(PyExc_TypeError, "Palette expected"); return {};}
        val[0] = PyLong_AsUnsignedLong(rv);
        PyObject *gv = PyTuple_GetItem(v, 1);
        if (!PyLong_Check(gv)) {PyErr_SetString(PyExc_TypeError, "Palette expected"); return {};}
        val[1] = PyLong_AsUnsignedLong(gv);
        PyObject *bv = PyTuple_GetItem(v, 0);
        if (!PyLong_Check(bv)) {PyErr_SetString(PyExc_TypeError, "Palette expected"); return {};}
        val[2] = PyLong_AsUnsignedLong(bv);
        retval.push_back(val);
    }
    return retval;
}

static PyObject *NewPalette(const std::vector<Vec3b>& palette) {
    PyObject *retval = PyList_New(palette.size());
    for (int i = 0; i < palette.size(); i++) {
        PyObject *obj = PyTuple_New(3);
        PyTuple_SetItem(obj, 2, PyLong_FromUnsignedLong(palette[i][0]));
        PyTuple_SetItem(obj, 1, PyLong_FromUnsignedLong(palette[i][1]));
        PyTuple_SetItem(obj, 0, PyLong_FromUnsignedLong(palette[i][2]));
        PyList_SetItem(retval, i, obj);
    }
    return retval;
}

static PyObject *M_initOpenCL(PyObject *self, PyObject *args) {
#ifdef HAS_OPENCL
    if (device != NULL) return Py_True;
    try {
        OpenCL::Device_Info devinfo;
        std::vector<OpenCL::Device_Info> devices = OpenCL::get_devices(false);
        if (PyTuple_Size(args) > 0) {
            PyObject *arg = PyTuple_GetItem(args, 0);
            if (PyLong_Check(arg)) {
                devinfo = OpenCL::select_device_with_id(PyLong_AsInt(arg));
            } else if (PyUnicode_Check(arg)) {
                std::string str = PyUnicode_AsUTF8(arg);
                if (str == "best_flops") {
                    devinfo = OpenCL::select_device_with_most_flops(devices, false);
                } else if (str == "best_memory") {
                    devinfo = OpenCL::select_device_with_most_memory(devices, false);
                } else {
                    PyErr_SetString(PyExc_ValueError, "Invalid option for device");
                    return NULL;
                }
            } else {
                PyErr_SetString(PyExc_TypeError, "Number or string expected");
                return NULL;
            }
        } else {
            devinfo = OpenCL::select_device_with_most_flops(devices, false);
        }
        device = new OpenCL::Device(devinfo);
    } catch (const OpenCL::OpenCLException& e) {
        fprintf(stderr, "Failed to initialize OpenCL: %s\n", e.what());
        return Py_False;
    }
    return Py_True;
#else
    fprintf(stderr, "Failed to initialize OpenCL: Feature not available\n");
    return Py_False;
#endif
}

static PyObject *M_makeRGBImage(PyObject *self, PyObject *args) {
    if (PyTuple_Size(args) == 0) {
        PyErr_SetString(PyExc_TypeError, "Object expected");
        return NULL;
    }
    PyObject *arg = PyTuple_GetItem(args, 0);
    Mat * img = NULL;
    if (PyList_Check(arg)) {
        unsigned height = PyList_Size(arg);
        if (height == 0) {PyErr_SetString(PyExc_ValueError, "Image has no data"); return NULL;}
        PyObject *first = PyList_GetItem(arg, 0);
        if (first == NULL) {PyErr_SetString(PyExc_ValueError, "Image has no data"); return NULL;}
        else if (PyList_Check(first)) {
            // Color[][]/(number, number, number)[][]
            unsigned width = PyList_Size(first);
            for (unsigned y = 1; y < height; y++) {
                PyObject *row = PyList_GetItem(arg, y);
                if (!PyList_Check(row) || PyList_Size(row) != width) {PyErr_SetString(PyExc_TypeError, "Invalid pixel array"); return NULL;}
            }
            img = new Mat(width, height, device);
            for (unsigned y = 0; y < height; y++) {
                PyObject *row = PyList_GetItem(arg, y);
                Mat::row imgrow = (*img)[y];
                for (unsigned x = 0; x < width; x++) {
                    PyObject *color = PyList_GetItem(row, x);
                    if (PyTuple_Check(color)) {
                        if (PyTuple_Size(color) != 3) {
                            delete img;
                            {PyErr_SetString(PyExc_TypeError, "Invalid pixel array"); return NULL;}
                        }
                        PyObject *r = PyTuple_GetItem(color, 0);
                        if (!PyLong_Check(r)) {
                            delete img;
                            {PyErr_SetString(PyExc_TypeError, "Invalid pixel array"); return NULL;}
                        }
                        imgrow[x].z = PyLong_AsUnsignedLong(r);
                        PyObject *g = PyTuple_GetItem(color, 1);
                        if (!PyLong_Check(g)) {
                            delete img;
                            {PyErr_SetString(PyExc_TypeError, "Invalid pixel array"); return NULL;}
                        }
                        imgrow[x].y = PyLong_AsUnsignedLong(g);
                        PyObject *b = PyTuple_GetItem(color, 2);
                        if (!PyLong_Check(b)) {
                            delete img;
                            {PyErr_SetString(PyExc_TypeError, "Invalid pixel array"); return NULL;}
                        }
                        imgrow[x].x = PyLong_AsUnsignedLong(b);
                    } else if (PyDict_Check(color)) {
                        PyObject *r = PyDict_GetItemString(color, "r");
                        if (r == NULL || !PyLong_Check(r)) {
                            delete img;
                            {PyErr_SetString(PyExc_TypeError, "Invalid pixel array"); return NULL;}
                        }
                        imgrow[x].z = PyLong_AsUnsignedLong(r);
                        PyObject *g = PyDict_GetItemString(color, "g");
                        if (g == NULL || !PyLong_Check(g)) {
                            delete img;
                            {PyErr_SetString(PyExc_TypeError, "Invalid pixel array"); return NULL;}
                        }
                        imgrow[x].y = PyLong_AsUnsignedLong(g);
                        PyObject *b = PyDict_GetItemString(color, "b");
                        if (b == NULL || !PyLong_Check(b)) {
                            delete img;
                            {PyErr_SetString(PyExc_TypeError, "Invalid pixel array"); return NULL;}
                        }
                        imgrow[x].x = PyLong_AsUnsignedLong(b);
                    } else {
                        delete img;
                        {PyErr_SetString(PyExc_TypeError, "Invalid pixel array"); return NULL;}
                    }
                }
            }
        } else if (PyLong_Check(first)) {
            // uint32_t[]
            unsigned width, height;
            const char * format;
            if (!PyArg_ParseTuple(args, "OIIs", &arg, &width, &height, &format)) return NULL;
            img = new Mat(width, height, device);
            int tp;
            if (strcmp(format, "rgba") == 0) tp = 0;
            else if (strcmp(format, "argb") == 0) tp = 1;
            else if (strcmp(format, "bgra") == 0) tp = 2;
            else if (strcmp(format, "abgr") == 0) tp = 3;
            else {
                delete img;
                {PyErr_SetString(PyExc_TypeError, "Invalid format specification"); return NULL;}
            }
            for (unsigned y = 0; y < height; y++) {
                Mat::row row = (*img)[y];
                for (unsigned x = 0; x < width; x++) {
                    unsigned pos = y * width + x;
                    if (pos >= PyList_Size(arg)) {
                        delete img;
                        {PyErr_SetString(PyExc_ValueError, "Image data too short for specified size"); return NULL;}
                    }
                    PyObject *c = PyList_GetItem(arg, pos);
                    if (c == NULL || !PyLong_Check(c)) {
                        delete img;
                        {PyErr_SetString(PyExc_ValueError, "Invalid pixel array"); return NULL;}
                    }
                    uint32_t d = PyLong_AsUnsignedLong(c);
                    switch (tp) {
                        case 0: row[x] = {(uint8_t)((d >> 8) & 0xFF), (uint8_t)((d >> 16) & 0xFF), (uint8_t)((d >> 24) & 0xFF)}; break;
                        case 1: row[x] = {(uint8_t)((d) & 0xFF), (uint8_t)((d >> 8) & 0xFF), (uint8_t)((d >> 16) & 0xFF)}; break;
                        case 2: row[x] = {(uint8_t)((d >> 24) & 0xFF), (uint8_t)((d >> 16) & 0xFF), (uint8_t)((d >> 8) & 0xFF)}; break;
                        case 3: row[x] = {(uint8_t)((d >> 16) & 0xFF), (uint8_t)((d >> 8) & 0xFF), (uint8_t)((d) & 0xFF)}; break;
                    }
                }
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid pixel array"); return NULL;
        }
    } else if (PyByteArray_Check(arg)) {
        // bytearray
        unsigned width, height;
        const char * format;
        if (!PyArg_ParseTuple(args, "OIIs", &arg, &width, &height, &format)) return NULL;
        img = new Mat(width, height, device);
        const uint8_t* data = (uint8_t*)PyByteArray_AsString(arg);
        if (strcmp(format, "rgb") == 0 || strcmp(format, "bgr") == 0) {
            bool bgr = strcmp(format, "bgr") == 0;
            for (unsigned y = 0; y < height; y++) {
                Mat::row row = (*img)[y];
                for (unsigned x = 0; x < width; x++) {
                    unsigned pos = (y * width + x) * 3;
                    if (pos + 2 >= PyByteArray_Size(arg)) {
                        delete img;
                        {PyErr_SetString(PyExc_ValueError, "Image data too short for specified size"); return NULL;}
                    }
                    if (!bgr) row[x] = {data[pos + 2], data[pos + 1], data[pos]};
                    else row[x] = {data[pos], data[pos + 1], data[pos + 2]};
                }
            }
        } else {
            int tp;
            if (strcmp(format, "rgba") == 0) tp = 0;
            else if (strcmp(format, "argb") == 0) tp = 1;
            else if (strcmp(format, "bgra") == 0) tp = 2;
            else if (strcmp(format, "abgr") == 0) tp = 3;
            else {
                delete img;
                {PyErr_SetString(PyExc_TypeError, "Invalid format specification"); return NULL;}
            }
            for (unsigned y = 0; y < height; y++) {
                Mat::row row = (*img)[y];
                for (unsigned x = 0; x < width; x++) {
                    unsigned pos = (y * width + x) * 4;
                    if (pos + 3 >= PyByteArray_Size(arg)) {
                        delete img;
                        {PyErr_SetString(PyExc_ValueError, "Image data too short for specified size"); return NULL;}
                    }
                    switch (tp) {
                        case 0: row[x] = {data[pos + 2], data[pos + 1], data[pos]}; break;
                        case 1: row[x] = {data[pos + 3], data[pos + 2], data[pos + 1]}; break;
                        case 2: row[x] = {data[pos], data[pos + 1], data[pos + 2]}; break;
                        case 3: row[x] = {data[pos + 1], data[pos + 2], data[pos + 3]}; break;
                    }
                }
            }
        }
    } else if (PyBytes_Check(arg)) {
        // bytes
        unsigned width, height;
        const char * format;
        if (!PyArg_ParseTuple(args, "OIIs", &arg, &width, &height, &format)) return NULL;
        img = new Mat(width, height, device);
        const uint8_t* data = (uint8_t*)PyBytes_AsString(arg);
        if (strcmp(format, "rgb") == 0 || strcmp(format, "bgr") == 0) {
            bool bgr = strcmp(format, "bgr") == 0;
            for (unsigned y = 0; y < height; y++) {
                Mat::row row = (*img)[y];
                for (unsigned x = 0; x < width; x++) {
                    unsigned pos = (y * width + x) * 3;
                    if (pos + 2 >= PyBytes_Size(arg)) {
                        delete img;
                        {PyErr_SetString(PyExc_ValueError, "Image data too short for specified size"); return NULL;}
                    }
                    if (!bgr) row[x] = {data[pos + 2], data[pos + 1], data[pos]};
                    else row[x] = {data[pos], data[pos + 1], data[pos + 2]};
                }
            }
        } else {
            int tp;
            if (strcmp(format, "rgba") == 0) tp = 0;
            else if (strcmp(format, "argb") == 0) tp = 1;
            else if (strcmp(format, "bgra") == 0) tp = 2;
            else if (strcmp(format, "abgr") == 0) tp = 3;
            else {
                delete img;
                {PyErr_SetString(PyExc_TypeError, "Invalid format specification"); return NULL;}
            }
            for (unsigned y = 0; y < height; y++) {
                Mat::row row = (*img)[y];
                for (unsigned x = 0; x < width; x++) {
                    unsigned pos = (y * width + x) * 4;
                    if (pos + 3 >= PyBytes_Size(arg)) {
                        delete img;
                        {PyErr_SetString(PyExc_ValueError, "Image data too short for specified size"); return NULL;}
                    }
                    switch (tp) {
                        case 0: row[x] = {data[pos + 2], data[pos + 1], data[pos]}; break;
                        case 1: row[x] = {data[pos + 3], data[pos + 2], data[pos + 1]}; break;
                        case 2: row[x] = {data[pos], data[pos + 1], data[pos + 2]}; break;
                        case 3: row[x] = {data[pos + 1], data[pos + 2], data[pos + 3]}; break;
                    }
                }
            }
        }
    } else {
        {PyErr_SetString(PyExc_TypeError, "Object expected"); return NULL;}
    }

    return NewRGBImage(img);
}

static PyObject *M_makeLabImage(PyObject *self, PyObject *args) {
    PyObject *arg;
    if (!PyArg_ParseTuple(args, "O", &arg)) return NULL;
    Mat *img = GetRGBImage(arg);
    if (img == NULL) return NULL;
    return NewRGBImage(new Mat(makeLabImage(*img, device)));
}

static PyObject *M_convertLabPalette(PyObject *self, PyObject *args) {
    PyObject *arg;
    if (!PyArg_ParseTuple(args, "O", &arg)) return NULL;
    std::vector<Vec3b> palette = GetPalette(arg);
    if (palette.empty()) return NULL;
    return NewPalette(convertLabPalette(palette));
}

static PyObject *M_reducePalette_medianCut(PyObject *self, PyObject *args) {
    PyObject *arg;
    int numColors = 16;
    if (!PyArg_ParseTuple(args, "O|i", &arg, &numColors)) return NULL;
    Mat *img = GetRGBImage(arg);
    if (img == NULL) return NULL;
    return NewPalette(reducePalette_medianCut(*img, numColors, device));
}

static PyObject *M_reducePalette_kMeans(PyObject *self, PyObject *args) {
    PyObject *arg;
    int numColors = 16;
    if (!PyArg_ParseTuple(args, "O|i", &arg, &numColors)) return NULL;
    Mat *img = GetRGBImage(arg);
    if (img == NULL) return NULL;
    return NewPalette(reducePalette_kMeans(*img, numColors, device));
}

static PyObject *M_reducePalette_octree(PyObject *self, PyObject *args) {
    PyObject *arg;
    int numColors = 16;
    if (!PyArg_ParseTuple(args, "O|i", &arg, &numColors)) return NULL;
    Mat *img = GetRGBImage(arg);
    if (img == NULL) return NULL;
    return NewPalette(reducePalette_octree(*img, numColors, device));
}

static PyObject *M_thresholdImage(PyObject *self, PyObject *args) {
    PyObject *arg1, *arg2;
    if (!PyArg_ParseTuple(args, "OO", &arg1, &arg2)) return NULL;
    Mat *img = GetRGBImage(arg1);
    if (img == NULL) return NULL;
    std::vector<Vec3b> palette = GetPalette(arg2);
    if (palette.empty()) return NULL;
    Mat res = thresholdImage(*img, palette, device);
    return NewIndexedImage(new Mat1b(rgbToPaletteImage(res, palette, device)));
}

static PyObject *M_ditherImage_ordered(PyObject *self, PyObject *args) {
    PyObject *arg1, *arg2;
    if (!PyArg_ParseTuple(args, "OO", &arg1, &arg2)) return NULL;
    Mat *img = GetRGBImage(arg1);
    if (img == NULL) return NULL;
    std::vector<Vec3b> palette = GetPalette(arg2);
    if (palette.empty()) return NULL;
    Mat res = ditherImage_ordered(*img, palette, device);
    return NewIndexedImage(new Mat1b(rgbToPaletteImage(res, palette, device)));
}

static PyObject *M_ditherImage_floydSteinberg(PyObject *self, PyObject *args) {
    PyObject *arg1, *arg2;
    if (!PyArg_ParseTuple(args, "OO", &arg1, &arg2)) return NULL;
    Mat *img = GetRGBImage(arg1);
    if (img == NULL) return NULL;
    std::vector<Vec3b> palette = GetPalette(arg2);
    if (palette.empty()) return NULL;
    Mat res = ditherImage(*img, palette, device);
    return NewIndexedImage(new Mat1b(rgbToPaletteImage(res, palette, device)));
}

static PyObject *M_makeTable(PyObject *self, PyObject *args) {
    PyObject *arg1, *arg2;
    bool compact = false, embedPalette = false, binary = false;
    if (!PyArg_ParseTuple(args, "OO|ppp", &arg1, &arg2, &compact, &embedPalette, &binary)) return NULL;
    Mat1b *img = GetIndexedImage(arg1);
    if (img == NULL) return NULL;
    std::vector<Vec3b> palette = GetPalette(arg2);
    if (palette.empty()) return NULL;
    uchar *chars, *cols;
    makeCCImage(*img, palette, &chars, &cols, device);
    std::string retval = makeTable(chars, cols, palette, img->width / 2, img->height / 3, compact, embedPalette, binary);
    delete[] chars;
    delete[] cols;
    return PyUnicode_FromStringAndSize(retval.c_str(), retval.size());
}

static PyObject *M_makeNFP(PyObject *self, PyObject *args) {
    PyObject *arg1, *arg2;
    if (!PyArg_ParseTuple(args, "OO", &arg1, &arg2)) return NULL;
    Mat1b *img = GetIndexedImage(arg1);
    if (img == NULL) return NULL;
    std::vector<Vec3b> palette = GetPalette(arg2);
    if (palette.empty()) return NULL;
    uchar *chars, *cols;
    makeCCImage(*img, palette, &chars, &cols, device);
    std::string retval = makeNFP(chars, cols, palette, img->width / 2, img->height / 3);
    delete[] chars;
    delete[] cols;
    return PyUnicode_FromStringAndSize(retval.c_str(), retval.size());
}

static PyObject *M_makeLuaFile(PyObject *self, PyObject *args) {
    PyObject *arg1, *arg2;
    if (!PyArg_ParseTuple(args, "OO", &arg1, &arg2)) return NULL;
    Mat1b *img = GetIndexedImage(arg1);
    if (img == NULL) return NULL;
    std::vector<Vec3b> palette = GetPalette(arg2);
    if (palette.empty()) return NULL;
    uchar *chars, *cols;
    makeCCImage(*img, palette, &chars, &cols, device);
    std::string retval = makeLuaFile(chars, cols, palette, img->width / 2, img->height / 3);
    delete[] chars;
    delete[] cols;
    return PyUnicode_FromStringAndSize(retval.c_str(), retval.size());
}

static PyObject *M_makeRawImage(PyObject *self, PyObject *args) {
    PyObject *arg1, *arg2;
    if (!PyArg_ParseTuple(args, "OO", &arg1, &arg2)) return NULL;
    Mat1b *img = GetIndexedImage(arg1);
    if (img == NULL) return NULL;
    std::vector<Vec3b> palette = GetPalette(arg2);
    if (palette.empty()) return NULL;
    uchar *chars, *cols;
    makeCCImage(*img, palette, &chars, &cols, device);
    std::string retval = makeRawImage(chars, cols, palette, img->width / 2, img->height / 3);
    delete[] chars;
    delete[] cols;
    return PyUnicode_FromStringAndSize(retval.c_str(), retval.size());
}

static PyObject *M_make32vid(PyObject *self, PyObject *args) {
    PyObject *arg1, *arg2;
    if (!PyArg_ParseTuple(args, "OO", &arg1, &arg2)) return NULL;
    Mat1b *img = GetIndexedImage(arg1);
    if (img == NULL) return NULL;
    std::vector<Vec3b> palette = GetPalette(arg2);
    if (palette.empty()) return NULL;
    uchar *chars, *cols;
    makeCCImage(*img, palette, &chars, &cols, device);
    std::string retval = make32vid(chars, cols, palette, img->width / 2, img->height / 3);
    delete[] chars;
    delete[] cols;
    return PyBytes_FromStringAndSize(retval.c_str(), retval.size());
}

static PyObject *M_make32vid_cmp(PyObject *self, PyObject *args) {
    PyObject *arg1, *arg2;
    if (!PyArg_ParseTuple(args, "OO", &arg1, &arg2)) return NULL;
    Mat1b *img = GetIndexedImage(arg1);
    if (img == NULL) return NULL;
    std::vector<Vec3b> palette = GetPalette(arg2);
    if (palette.empty()) return NULL;
    uchar *chars, *cols;
    makeCCImage(*img, palette, &chars, &cols, device);
    std::string retval = make32vid_cmp(chars, cols, palette, img->width / 2, img->height / 3);
    delete[] chars;
    delete[] cols;
    return PyBytes_FromStringAndSize(retval.c_str(), retval.size());
}

static PyObject *M_make32vid_ans(PyObject *self, PyObject *args) {
    PyObject *arg1, *arg2;
    if (!PyArg_ParseTuple(args, "OO", &arg1, &arg2)) return NULL;
    Mat1b *img = GetIndexedImage(arg1);
    if (img == NULL) return NULL;
    std::vector<Vec3b> palette = GetPalette(arg2);
    if (palette.empty()) return NULL;
    uchar *chars, *cols;
    makeCCImage(*img, palette, &chars, &cols, device);
    std::string retval = make32vid_ans(chars, cols, palette, img->width / 2, img->height / 3);
    delete[] chars;
    delete[] cols;
    return PyBytes_FromStringAndSize(retval.c_str(), retval.size());
}


#define addFunction(name, description) {#name, M_##name, METH_VARARGS, description}

static PyMethodDef sanjuuni_module_methods[] = {
    addFunction(initOpenCL, "Initializes OpenCL support if available."),
    addFunction(makeRGBImage, "Creates a new RGBImage from pixel data."),
    addFunction(makeLabImage, "Converts an RGBImage to Lab color space."),
    addFunction(convertLabPalette, "Converts a Lab palette to RGB. Use this before generating output images."),
    addFunction(reducePalette_medianCut, "Creates an optimized palette from an RGBImage using the median cut algorithm."),
    addFunction(reducePalette_kMeans, "Creates an optimized palette from an RGBImage using the k-means algorithm."),
    addFunction(reducePalette_octree, "Creates an optimized palette from an RGBImage using the octree algorithm."),
    addFunction(thresholdImage, "Converts an RGBImage into an IndexedImage without dithering."),
    addFunction(ditherImage_ordered, "Converts an RGBImage into an IndexedImage with ordered dithering."),
    addFunction(ditherImage_floydSteinberg, "Converts an RGBImage into an IndexedImage with Floyd-Steinberg dithering."),
    addFunction(makeTable, "Generates a Lua table in BIMG frame format from an IndexedImage."),
    addFunction(makeNFP, "Generates an NFP file from an IndexedImage."),
    addFunction(makeLuaFile, "Generates a Lua executable program from an IndexedImage."),
    addFunction(makeRawImage, "Generates a raw mode format frame from an IndexedImage."),
    addFunction(make32vid, "Generates a 32vid frame from an IndexedImage."),
    addFunction(make32vid_cmp, "Generates a 32vid frame with compression from an IndexedImage."),
    addFunction(make32vid_ans, "Generates a 32vid frame with ANS compression from an IndexedImage."),
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef sanjuuni_module = {
    PyModuleDef_HEAD_INIT,
    "sanjuuni",
    "Converts images and videos into a format suitable for ComputerCraft, based on sanjuuni.",
    0,
    sanjuuni_module_methods
};

PyMODINIT_FUNC PyInit_sanjuuni() {
    PyType_Ready(&RGBImage);
    PyType_Ready(&IndexedImage);
    return PyModuleDef_Init(&sanjuuni_module);
}
