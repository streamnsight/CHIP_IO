/*
Copyright (c) 2016 Robert Wolterman

Original BBIO Author Justin Cooper
Modified for CHIP_IO Author Robert Wolterman

This file incorporates work covered by the following copyright and
permission notice, all modified code adopts the original license:

Copyright (c) 2013 Adafruit
Author: Justin Cooper

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Python.h"
#include "constants.h"
#include "common.h"
#include "c_pwm.h"

// python function cleanup()
static PyObject *py_cleanup(PyObject *self, PyObject *args)
{
    // unexport the PWM
    pwm_cleanup();

    Py_RETURN_NONE;
}

// python function start(channel, duty_cycle, freq)
static PyObject *py_start_channel(PyObject *self, PyObject *args, PyObject *kwargs)
{
    char key[8];
    char *channel;
    float frequency = 2000.0;
    float duty_cycle = 0.0;
    int polarity = 0;
    static char *kwlist[] = {"channel", "duty_cycle", "frequency", "polarity", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|ffi", kwlist, &channel, &duty_cycle, &frequency, &polarity)) {
        return NULL;
    }

    if (!get_pwm_key(channel, key)) {
        PyErr_SetString(PyExc_ValueError, "Invalid PWM key or name.");
        return NULL;
    }

    if (duty_cycle < 0.0 || duty_cycle > 100.0)
    {
        PyErr_SetString(PyExc_ValueError, "duty_cycle must have a value from 0.0 to 100.0");
        return NULL;
    }

    if (frequency <= 0.0)
    {
        PyErr_SetString(PyExc_ValueError, "frequency must be greater than 0.0");
        return NULL;
    }

    if (polarity < 0 || polarity > 1) {
        PyErr_SetString(PyExc_ValueError, "polarity must be either 0 or 1");
        return NULL;
    }

    if (!pwm_start(key, duty_cycle, frequency, polarity))
        return NULL;

    Py_RETURN_NONE;
}

// python function stop(channel)
static PyObject *py_stop_channel(PyObject *self, PyObject *args, PyObject *kwargs)
{
    char key[8];
    char *channel;

    if (!PyArg_ParseTuple(args, "s", &channel))
        return NULL;

    if (!get_pwm_key(channel, key)) {
        PyErr_SetString(PyExc_ValueError, "Invalid PWM key or name.");
        return NULL;
    }

    pwm_disable(key);

    Py_RETURN_NONE;
}

// python method PWM.set_duty_cycle(channel, duty_cycle)
static PyObject *py_set_duty_cycle(PyObject *self, PyObject *args, PyObject *kwargs)
{
    char key[8];
    char *channel;
    float duty_cycle = 0.0;
    static char *kwlist[] = {"channel", "duty_cycle", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|f", kwlist, &channel, &duty_cycle))
        return NULL;

    if (duty_cycle < 0.0 || duty_cycle > 100.0)
    {
        PyErr_SetString(PyExc_ValueError, "duty_cycle must have a value from 0.0 to 100.0");
        return NULL;
    }

    if (!get_pwm_key(channel, key)) {
        PyErr_SetString(PyExc_ValueError, "Invalid PWM key or name.");
        return NULL;
    }

    if (pwm_set_duty_cycle(key, duty_cycle) == -1) {
        PyErr_SetString(PyExc_RuntimeError, "You must start() the PWM channel first");
        return NULL;
    }

    Py_RETURN_NONE;
}

// python method PWM.set_pulse_width(channel, pulse_width_ns)
static PyObject *py_set_pulse_width_ns(PyObject *self, PyObject *args, PyObject *kwargs)
{
    char key[8];
    char *channel;
    unsigned long pulse_width_ns = 0.0;
    static char *kwlist[] = {"channel", "pulse_width_ns", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|k", kwlist, &channel, &pulse_width_ns))
        return NULL;

    pwm = lookup_exported_pwm(key);

    if (pwm == NULL) {
        PyErr_SetString(PyExc_ValueError, "Invalid PWM key or name.");
        return NULL;
    }

	if (pulse_width_ns < 0.0 || pulse_width_ns > pwm->period_ns)
    {
        PyErr_SetString(PyExc_ValueError, "pulse width must have a value from 0 to period");
        return NULL;
    }

    if (pwm_set_pulse_width_ns(key, duty_cycle) == -1) {
        PyErr_SetString(PyExc_RuntimeError, "You must start() the PWM channel first");
        return NULL;
    }

    Py_RETURN_NONE;
}

// python method PWM.set_frequency(channel, frequency)
static PyObject *py_set_frequency(PyObject *self, PyObject *args, PyObject *kwargs)
{
    char key[8];
    char *channel;
    float frequency = 1.0;
    static char *kwlist[] = {"channel", "frequency", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|f", kwlist, &channel, &frequency))
        return NULL;

    if (frequency <= 0.0)
    {
        PyErr_SetString(PyExc_ValueError, "frequency must be greater than 0.0");
        return NULL;
    }

    if (!get_pwm_key(channel, key)) {
        PyErr_SetString(PyExc_ValueError, "Invalid PWM key or name.");
        return NULL;
    }

    if (pwm_set_frequency(key, frequency) == -1) {
        PyErr_SetString(PyExc_RuntimeError, "You must start() the PWM channel first");
        return NULL;
    }

    Py_RETURN_NONE;
}

// python method PWM.set_period_ns(channel, period_ns)
static PyObject *py_set_period_ns(PyObject *self, PyObject *args, PyObject *kwargs)
{
    char key[8];
    char *channel;
    unsigned long period_ns = 2e6;
    static char *kwlist[] = {"channel", "period_ns", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|k", kwlist, &channel, &period_ns))
        return NULL;

    if (period_ns <= 0)
    {
        PyErr_SetString(PyExc_ValueError, "period must be greater than 0ns");
        return NULL;
    }

    if (!get_pwm_key(channel, key)) {
        PyErr_SetString(PyExc_ValueError, "Invalid PWM key or name.");
        return NULL;
    }

    if (pwm_set_period_ns(key, period_ns) == -1) {
        PyErr_SetString(PyExc_RuntimeError, "You must start() the PWM channel first");
        return NULL;
    }

    Py_RETURN_NONE;
}

static const char moduledocstring[] = "Hardware PWM functionality of a CHIP using Python";

PyMethodDef pwm_methods[] = {
    {"start", (PyCFunction)py_start_channel, METH_VARARGS | METH_KEYWORDS, "Set up and start the PWM channel.  channel can be in the form of 'PWM0', or 'U13_18'"},
    {"stop", (PyCFunction)py_stop_channel, METH_VARARGS | METH_KEYWORDS, "Stop the PWM channel.  channel can be in the form of 'PWM0', or 'U13_18'"},
    { "set_duty_cycle", (PyCFunction)py_set_duty_cycle, METH_VARARGS, "Change the duty cycle\ndutycycle - between 0.0 and 100.0" },
    { "set_frequency", (PyCFunction)py_set_frequency, METH_VARARGS, "Change the frequency\nfrequency - frequency in Hz (freq > 0.0)" },
    { "set_period_ns", (PyCFunction)py_set_period_ns, METH_VARARGS, "Change the period\nperiod_ns - period in nanoseconds" },
    { "set_pulse_width_ns", (PyCFunction)py_set_pulse_width_ns, METH_VARARGS, "Change the period\npulse_width_ns - pulse width in nanoseconds" },
    {"cleanup", py_cleanup, METH_VARARGS, "Clean up by resetting all GPIO channels that have been used by this program to INPUT with no pullup/pulldown and no event detection"},
    //{"setwarnings", py_setwarnings, METH_VARARGS, "Enable or disable warning messages"},
    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION > 2
static struct PyModuleDef chippwmmodule = {
    PyModuleDef_HEAD_INIT,
    "PWM",       // name of module
    moduledocstring,  // module documentation, may be NULL
    -1,               // size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
    pwm_methods
};
#endif

#if PY_MAJOR_VERSION > 2
PyMODINIT_FUNC PyInit_PWM(void)
#else
PyMODINIT_FUNC initPWM(void)
#endif
{
    PyObject *module = NULL;

#if PY_MAJOR_VERSION > 2
    if ((module = PyModule_Create(&chippwmmodule)) == NULL)
       return NULL;
#else
    if ((module = Py_InitModule3("PWM", pwm_methods, moduledocstring)) == NULL)
       return;
#endif

   define_constants(module);


#if PY_MAJOR_VERSION > 2
    return module;
#else
    return;
#endif
}
