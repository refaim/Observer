// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "stdafx.h"

#include "archive.h"

#include <cstdint>
#include <cstring>

#include <memory>
#include <utility>

#include <gsl/gsl>
#include <zlib.h>

#include "kriabal/kriabal.h"
#include "kriabal/streams.h"

namespace renpy
{
    enum ItemProps
    {
        OFFSET = 0,
        LENGTH = 1,
        PREFIX = 2,
    };

    void Archive::PrepareItems()
    {
        SkipSignature();
        stream_->Skip(strlen(" "));

        const int64_t index_offset = reader_->ReadSignedPositiveInt64FromHexString();
        const int64_t encryption_key = reader_->ReadSignedPositiveInt64FromHexString();

        auto pickled_index_bytes = std::make_unique<std::string>();
        {
            auto compressed_index_bytes = std::make_unique<std::string>(gsl::narrow<size_t>(stream_->GetSizeInBytes() - index_offset), '\0');

            stream_->Seek(index_offset);
            stream_->ReadBytes(*compressed_index_bytes.get(), compressed_index_bytes->size());

            UncompressZlibStream(*compressed_index_bytes.get(), *pickled_index_bytes.get());
        }

        auto pickled_index_stream = std::make_unique<kriabal::stream::MemoryStream>(pickled_index_bytes->size());
        pickled_index_stream->WriteBytes(pickled_index_bytes->c_str(), pickled_index_bytes->size());
        pickled_index_bytes.reset();
        UnpickleBytes(*pickled_index_stream->get());

        {
            auto python_context = python::Context();
            try
            {
                ParsePythonIndex(*index_raw_bytes.get(), encryption_key);
            }
            catch (python::NullReferenceError&)
            {
                throw kriabal::RuntimeError();
            }
        }
    }

    void Archive::UncompressZlibStream(const std::string& input_buffer, std::string& output_buffer)
    {
        const int32_t kStartingCompressionMultiplier = 4;

        int32_t compression_multiplier = kStartingCompressionMultiplier;
        uLongf uncompressed_length = 0;
        int32_t zlib_status = Z_BUF_ERROR;
        do
        {
            uncompressed_length = gsl::narrow<uLongf>(input_buffer.size() * gsl::narrow<size_t>(compression_multiplier));
            output_buffer.resize(gsl::narrow<size_t>(uncompressed_length));

            zlib_status = uncompress(reinterpret_cast<uint8_t*>(output_buffer.data()), &uncompressed_length, reinterpret_cast<const uint8_t*>(input_buffer.c_str()), gsl::narrow<uLong>(input_buffer.size()));

            ++compression_multiplier;
        }
        while (zlib_status == Z_BUF_ERROR);

        if (zlib_status != Z_OK)
            throw kriabal::RuntimeError();

        output_buffer.resize(gsl::narrow<size_t>(uncompressed_length));
    }

    // TODO rename !!!!
    void Archive::UnpickleBytes(kriabal::stream::MemoryStream& stream, std::vector<std::unique_ptr<ArchiveItem>>& items)
    {

    }

    void Archive::ParsePythonIndex(const std::string& input_buffer, int64_t encryption_key)
    {


        //auto ref_pickle_module_name = python::MakeStrongRef(PyUnicode_FromString("pickle"));
        //auto ref_pickle_module = python::MakeStrongRef(PyImport_Import(python::RawPtr(ref_pickle_module_name)));
        //auto ref_pickle_loader = python::MakeStrongRef(PyObject_GetAttrString(python::RawPtr(ref_pickle_module), "loads"));

        //auto ref_pickled_index_bytes = python::MakeWeakRef(PyByteArray_FromStringAndSize(input_buffer.c_str(), input_buffer.size()));
        //auto ref_pickle_loader_args = python::MakeWeakRef(PyTuple_New(1));
        //Assert(PyTuple_SetItem(python::RawPtr(ref_pickle_loader_args), 0, python::RawPtr(ref_pickled_index_bytes)) == 0);

        //auto ref_pickle_loader_kwargs = python::MakeStrongRef(PyDict_New());
        //auto ref_pickle_encoding = python::MakeWeakRef(PyUnicode_FromString("latin1"));
        //Assert(PyDict_SetItemString(python::RawPtr(ref_pickle_loader_kwargs), "encoding", python::RawPtr(ref_pickle_encoding)) == 0);

        //auto ref_index_dict = python::MakeStrongRef(PyObject_Call(python::RawPtr(ref_pickle_loader), python::RawPtr(ref_pickle_loader_args), python::RawPtr(ref_pickle_loader_kwargs)));
        //ReserveItems(PyDict_Size(python::RawPtr(ref_index_dict)));

        //PyObject* py_ptr_item_path = nullptr;
        //PyObject* py_ptr_item_props = nullptr;
        //Py_ssize_t item_index = 0;
        //while (PyDict_Next(python::RawPtr(ref_index_dict), &item_index, &py_ptr_item_path, &py_ptr_item_props))
        //{
        //    auto item = std::make_unique<ArchiveItem>();

        //    Assert(PyList_Size(py_ptr_item_props) == 1); // Not implemented yet
        //    auto ref_item_props = python::MakeWeakRef(PyList_GetItem(py_ptr_item_props, 0));

        //    auto ref_offset = python::MakeWeakRef(PyTuple_GetItem(python::RawPtr(ref_item_props), ItemProps::OFFSET));
        //    const int64_t offset = PyLong_AsLongLong(python::RawPtr(ref_offset));
        //    Assert(offset != -1);
        //    item->offset = offset ^ encryption_key;

        //    auto ref_size_in_bytes = python::MakeWeakRef(PyTuple_GetItem(python::RawPtr(ref_item_props), ItemProps::LENGTH));
        //    const int64_t size_in_bytes = PyLong_AsLongLong(python::RawPtr(ref_size_in_bytes));
        //    Assert(size_in_bytes != -1);
        //    item->size_in_bytes = size_in_bytes ^ encryption_key;

        //    Py_ssize_t item_path_raw_bytes_size = 0;
        //    auto item_path_raw_bytes = PyUnicode_AsUTF8AndSize(py_ptr_item_path, &item_path_raw_bytes_size);
        //    if (item_path_raw_bytes == nullptr || item_path_raw_bytes_size == 0)
        //        throw kriabal::RuntimeError();
        //    item->path.assign(item_path_raw_bytes, item_path_raw_bytes_size);

        //    if (PyTuple_Size(python::RawPtr(ref_item_props)) == 3)
        //    {
        //        auto ref_prefix_bytes_unicode = python::MakeWeakRef(PyTuple_GetItem(python::RawPtr(ref_item_props), ItemProps::PREFIX));
        //        auto ref_prefix_bytes_ansi = python::MakeStrongRef(PyUnicode_AsLatin1String(python::RawPtr(ref_prefix_bytes_unicode)));
        //        const auto prefix_size_in_bytes = PyBytes_Size(python::RawPtr(ref_prefix_bytes_ansi));
        //        if (prefix_size_in_bytes > 0)
        //        {
        //            auto prefix_raw_bytes = PyBytes_AsString(python::RawPtr(ref_prefix_bytes_ansi));
        //            if (prefix_raw_bytes == nullptr)
        //                throw kriabal::RuntimeError();
        //            item->header.assign(prefix_raw_bytes);
        //        }
        //    }

        //    PushItem(std::move(item));
        //}
    }
}
