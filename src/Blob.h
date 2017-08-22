/** @file */

#ifndef __SQLITEXX_SQLITE_BLOB_H__
#define __SQLITEXX_SQLITE_BLOB_H__

#include <sqlite3.h>

#include <cassert>
#include <cstring>
#include <memory>
#include <utility>


namespace SQLite
{
    /** A "Binary Large OBject".
     * A collection of binary data stored as a single entity in a database management system.
     * Blobs are typically images, audo or other multimedia object though they can be any form of data.
     */
    class Blob
    {
        public:
        /** Constructs a Blob object with contents of data.
         * @param[in] data the information you want the Blob to contain
         * @param[in] size the size in bytes of the data
         */
        Blob(const void* data, size_t size);

        /** Copy constructor.
         * Constructs a Blob object with a copy of the contents of other
         * @param[in] other another Blob object to use as source to initialize object with
         */
        Blob(const Blob& other);

        /** Move constructor.
         * Constructs a Blob object with a copy of the contents of other using move semantics
         * @param[in] other another Blob object to use as source to initialize object with
         */
        Blob(Blob&& other);

        /** Copy assignment operator.
         * Replaces the contents with those of other
         * @param[in] other another Blob object to use as source to initialize object with
         * @returns *this
         */
        Blob& operator=(const Blob& other);

        /** Move assignment operator.
         * Replaces the contents with those of other using move semantics
         * @param[in] other another Blob object to use as source to initialize object with
         * @returns *this
         */
        Blob& operator=(Blob&& other);

        /** The raw data of the Blob's contents.
         * @returns The raw data that the Blob object is storing.
         */
        const void* data() const;

        /** Used to get the size of the contained 'blob'.
         * @returns The size in bytes of the contained 'blob'.
         */
        size_t size() const;

        private:
        std::unique_ptr<char[]> m_data;
        size_t m_size;
    };
}


#endif
