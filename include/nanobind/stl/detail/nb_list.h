#include <nanobind/nanobind.h>

NAMESPACE_BEGIN(NB_NAMESPACE)
NAMESPACE_BEGIN(detail)

template <typename Value_, typename Entry> struct list_caster {
    NB_TYPE_CASTER(Value_, const_name("List[") + make_caster<Entry>::Name +
                               const_name("]"));

    using Caster = make_caster<Entry>;

    bool from_python(handle src, uint8_t flags, cleanup_list *cleanup) noexcept {
        size_t size = 0;
        PyObject *temp = nullptr;
        PyObject **o = seq_get(src.ptr(), &size, &temp);

        value.clear();
        value.reserve(size);

        Caster caster;
        bool success = true;

        for (size_t i = 0; i < size; ++i) {
            if (!caster.from_python(o[i], flags, cleanup)) {
                success = false;
                break;
            }
            value.push_back(((Caster &&) caster).operator cast_t<Entry &&>());
        }

        Py_XDECREF(temp);

        return success;
    }

    template <typename T>
    static handle from_cpp(T &&src, rv_policy policy, cleanup_list *cleanup) {
        object list = steal(PyList_New(src.size()));
        if (list) {
            Py_ssize_t index = 0;

            for (auto &value : src) {
                handle h =
                    Caster::from_cpp(forward_like<T>(value), policy, cleanup);

                PyList_SET_ITEM(list.ptr(), index++, h.ptr());
                if (!h.is_valid())
                    return handle();
            }
        }

        return list.release();
    }
};

NAMESPACE_END(detail)
NAMESPACE_END(NB_NAMESPACE)
