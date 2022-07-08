#define PY_SSIZE_T_CLEAN
#include <Python.h>
#define NPY_NO_DEPRECATED_API NPY_1_22_API_VERSION
#include <numpy/arrayobject.h>
#include "../../src/main.c"
#include "../../src/util/array.c"
#include "../../src/util/peak.c"
#include "../../src/util/bin.c"

static bool
is_pyobject_a_matrix(PyObject *arr)
{
    if (!PyArray_CheckExact(arr)) {
        PyErr_SetString(PyExc_RuntimeError, "not an array");
        return false;
    }
    // we are allowed to use a valid PyObject for these macros per numpy 1.23 docs
    if (PyArray_NDIM(arr) != 2) {
        PyErr_Format(PyExc_RuntimeError, "invalid number of dimension recieved %d (should be 2)", PyArray_NDIM(arr));
        return false;
    }
    if (PyArray_TYPE(arr) != NPY_DOUBLE) {
        PyErr_Format(PyExc_RuntimeError, "invalid data for array, should be double");
        return false;
    }
    if (!PyArray_ISBEHAVED(arr)) {
        PyErr_SetString(PyExc_RuntimeError, "invalid array, either not writable or not aligned");
        return false;
    }
    if (PyArray_STRIDES(arr)[1] != sizeof(double)) {
        PyErr_Format(PyExc_RuntimeError, "incorrect strides %zu (should be 0)", PyArray_STRIDES(arr)[1]);
        return false;
    }
    return true;
}

static struct matrix
mat_from_pyobject(PyObject *arr)
{
    // we are allowed to use a valid PyObject for these macros per numpy 1.23 docs
    double *data = PyArray_DATA(arr);
    npy_intp *dims = PyArray_DIMS(arr);
    npy_intp *strides = PyArray_STRIDES(arr);
    return mat_from_data(data, dims[0], dims[1], strides[0] / sizeof(double), false);
}

PyObject *
mat_to_pyobject(struct matrix m)
{
    // I read the numpy source code and `dims` get's memcpy'd so we can store it
    // on the stack (I was wrongfully worried `arr` would segfault if it
    // dereferences its dims)
    npy_intp dims[2];
    dims[0] = m.len1;
    dims[1] = m.len2;
    npy_intp strides[2];
    strides[0] = m.physlen * sizeof(double); // its measured in bytes
    strides[1] = sizeof(double); // measured by bytes of whole thing

    PyArray_Descr *descr = PyArray_DescrFromType(NPY_DOUBLE);

    // subtype: &PyArray_Type, first arg need to be that or a subtype of it
    // descr: specifies the data type
    // nd: 2 (number of dimensions)
    // dims: dims
    // strides: strides
    // data: m.data
    // flags: NPY_ARRAY_BEHAVED it's writable and aligned
    // NULL: this data is the object's, so no need to set a base pointer
    return PyArray_NewFromDescr(&PyArray_Type, descr, 2, dims, strides, m.data, NPY_ARRAY_BEHAVED, NULL);
}

static PyObject*
filenames_to_stats_parse(PyObject *dummy, PyObject *args, int mflag)
{
    const char *str;
    if (!PyArg_ParseTuple(args, "s", &str)) {
        PyErr_SetString(PyExc_RuntimeError, "didn't reviece a string");
        return NULL;
    }

    int len = strlen(str) + 1;
    char *copy = safe_calloc(len, 1);
    strlcpy(copy, str, len);
    struct matrix m = filenames_to_stats(copy, mflag);

    assert(m.is_owner);

    return mat_to_pyobject(m);
}

static PyObject*
filenames_to_stats_1d_cfunc(PyObject *dummy, PyObject *args)
{
    return filenames_to_stats_parse(dummy, args, ONED);
}

static PyObject*
filenames_to_stats_2d_cfunc(PyObject *dummy, PyObject *args)
{
    return filenames_to_stats_parse(dummy, args, TWOD);
}

static PyObject*
compare_compound_parse(PyObject *dummy, PyObject *args, int mflag)
{
    PyObject *arg1, *arg2;
    if (!PyArg_ParseTuple(args, "O!O!", &PyArray_Type, &arg1, &PyArray_Type, &arg2)) {
        PyErr_SetString(PyExc_RuntimeError, "didn't recieve arrays");
        return NULL;
    }

    if (!is_pyobject_a_matrix(arg1) || !is_pyobject_a_matrix(arg2)) {
        return NULL;
    }

    struct matrix m1 = mat_from_pyobject(arg1);
    struct matrix m2 = mat_from_pyobject(arg2);

    // make sure the right shape
    size_t second_dim = (mflag == ONED) ? 2 : 4;
    if (m1.len2 != second_dim) {
        PyErr_Format(PyExc_RuntimeError, "summary-statistics matrices must have shape (n, %zu) for %s resolution [first argument], got (%zu, %zu)", second_dim, (mflag == ONED) ? "low" : "high", m1.len1, m1.len2);
        return NULL;
    } else if (m2.len2 != second_dim) {
        PyErr_Format(PyExc_RuntimeError, "summary-statistics matrices must have shape (n, %zu) for %s resolution [second argument], got (%zu, %zu)", second_dim, (mflag == ONED) ? "low" : "high", m2.len1, m2.len2);
        return NULL;
    }

    double ret = compare_compound(m1, m2, mflag);
    mat_free(m1);
    mat_free(m2);
    return Py_BuildValue("d", ret);
}

static PyObject*
compare_compound_1d_cfunc(PyObject *dummy, PyObject *args)
{
    return compare_compound_parse(dummy, args, ONED);
}

static PyObject*
compare_compound_2d_cfunc(PyObject *dummy, PyObject *args)
{
    return compare_compound_parse(dummy, args, TWOD);
}

static PyObject*
compare_all_parse(PyObject *dummy, PyObject *args, int mflag)
{
    PyObject *obj;
    if (!PyArg_ParseTuple(args, "O", &obj)) {
        PyErr_SetString(PyExc_RuntimeError, "didn't recieve an object");
        return NULL;
    }

    PyObject *seq;
    const char *mes = "didn't get iterable/list/ndarray";
    if ((seq = PySequence_Fast(obj, mes)) == NULL) {
        return NULL;
    }

    Py_ssize_t len = PySequence_Fast_GET_SIZE(seq);
    PyObject **arrays = PySequence_Fast_ITEMS(seq);

    struct matarray matarr = matarr_zeros(len);
    for (Py_ssize_t i = 0; i < len; i++) {
        if (!is_pyobject_a_matrix(arrays[i])) {
            return NULL;
        }
        matarr_set(matarr, i, mat_from_pyobject(arrays[i]));
    }
    struct matrix mat = compare_all(matarr, mflag);
    return mat_to_pyobject(mat);
}


static PyObject*
compare_all_1d_cfunc(PyObject *dummy, PyObject *args)
{
    return compare_all_parse(dummy, args, ONED);
}

static PyObject*
compare_all_2d_cfunc(PyObject *dummy, PyObject *args)
{
    return compare_all_parse(dummy, args, TWOD);
}

static PyMethodDef mymethods[] = {
    {"filenames_to_stats_1d", filenames_to_stats_1d_cfunc, METH_VARARGS, 
      "takes a list of filenames for (high resolution) replicates, and returns the statistics for them all"},
    {"filenames_to_stats_2d", filenames_to_stats_2d_cfunc, METH_VARARGS, 
      "takes a list of filenames for (low resolution) replicates, and returns the statistics for them all"},
    {"compare_compound_1d", compare_compound_1d_cfunc, METH_VARARGS,
      "takes 2 summary-statistics arrays (output of filenames_to_stats_1d) returns the similarity of the stats"},
    {"compare_compound_2d", compare_compound_2d_cfunc, METH_VARARGS,
      "takes 2 summary-statistics arrays (output of filenames_to_stats_2d) returns the similarity of the stats"},
    {"compare_all_1d", compare_all_1d_cfunc, METH_VARARGS,
      "takes a list/array/sequence of summary-statistics arrays (output of filenames_to_stats_1d) returns a 2d array of comparisons"},
    {"compare_all_2d", compare_all_2d_cfunc, METH_VARARGS,
      "takes a list/array/sequence of summary-statistics arrays (output of filenames_to_stats_2d) returns a 2d array of comparisons"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static struct PyModuleDef spammodule = {
    PyModuleDef_HEAD_INIT,
    "hdcms",   /* name of module */
    NULL,      /* module documentation, may be NULL */
    -1,        /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    mymethods
};

PyMODINIT_FUNC
PyInit_hdcms(void)
{
    import_array()
    return PyModule_Create(&spammodule);
}

