/** @file */

#ifndef __SQLITEXX_SQLITE_BLOB_H__
#define __SQLITEXX_SQLITE_BLOB_H__

#include <sqlite3.h>

#include <cassert>
#include <cstring>
#include <memory>
#include <utility>


namespace sqlite
{
    /** A "Binary Large OBject".
     * A collection of binary data stored as a single entity in a database management system.
     * blobs are typically images, audo or other multimedia object though they can be any form of data.
     */
    class blob
    {
        public:
        /** Constructs a blob object with contents of data.
         * @param[in] data the information you want the blob to contain
         * @param[in] size the size in bytes of the data
         */
        blob(const void* data, const size_t size);

        /** Copy constructor.
         * Constructs a blob object with a copy of the contents of other
         * @param[in] other another blob object to use as source to initialize object with
         */
        blob(const blob& other);

        /** Move constructor.
         * Constructs a blob object with a copy of the contents of other using move semantics
         * @param[in] other another blob object to use as source to initialize object with
         */
        blob(blob&& other);

        /** Copy assignment operator.
         * Replaces the contents with those of other
         * @param[in] other another blob object to use as source to initialize object with
         * @returns *this
         */
        blob& operator=(const blob& other);

        /** Move assignment operator.
         * Replaces the contents with those of other using move semantics
         * @param[in] other another blob object to use as source to initialize object with
         * @returns *this
         */
        blob& operator=(blob&& other);

        /** The raw data of the blob's contents.
         * @returns The raw data that the blob object is storing.
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
