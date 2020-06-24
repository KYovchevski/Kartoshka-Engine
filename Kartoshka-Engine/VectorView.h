#pragma once

#include <cassert>
#include <vector>

namespace krt
{
    namespace hlp
    {
        // Helped class to allow viewing a vectors data in another format
        // without having to copy the vector

        template<typename ViewType, typename VectorType>
        class VectorView
        {
        public:
            VectorView(std::vector<VectorType>& a_Vector);
            ~VectorView();

            uint64_t Size();

            ViewType* operator[](uint64_t a_Index);

        private:
            std::vector<VectorType> m_Vector;
        };

        template <typename ViewType, typename VectorType>
        VectorView<ViewType, VectorType>::VectorView(std::vector<VectorType>& a_Vector)
            : m_Vector(a_Vector)
        {

        }

        template <typename ViewType, typename VectorType>
        VectorView<ViewType, VectorType>::~VectorView()
        {
        }

        template <typename ViewType, typename VectorType>
        uint64_t VectorView<ViewType, VectorType>::Size()
        {
            return (m_Vector.size() * sizeof(VectorType)) / sizeof(ViewType);
        }

        template <typename ViewType, typename VectorType>
        ViewType* VectorView<ViewType, VectorType>::operator[](uint64_t a_Index)
        {
            assert(a_Index < Size());
            ViewType* data = reinterpret_cast<ViewType*>(m_Vector.data());

            data += a_Index;

            return data;
        }
    }
}
