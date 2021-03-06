/**
 *  HMLP (High-Performance Machine Learning Primitives)
 *  
 *  Copyright (C) 2014-2017, The University of Texas at Austin
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see the LICENSE file.
 *
 **/  

#ifndef DATA_HPP
#define DATA_HPP

/** Use mmap as simple out-of-core solution. */
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

/** Use STL and inherit vector<T>. */
#include <cassert>
#include <typeinfo>
#include <algorithm>
#include <random>
#include <chrono>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <vector>
#include <deque>
#include <map>
#include <string>

/** std::istringstream */
#include <iostream>
#include <fstream>
#include <sstream>

/** Use HMLP support. */
#include <hmlp_device.hpp>
#include <hmlp_runtime.hpp>
#include <hmlp_util.hpp>


/** -lmemkind */
#ifdef HMLP_MIC_AVX512
#include <hbwmalloc.h>
#include <hbw_allocator.h>
#endif // ifdef HMLP}_MIC_AVX512

/** gpu related */
#ifdef HMLP_USE_CUDA
#include <hmlp_gpu.hpp>
#include <thrust/tuple.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/system/cuda/experimental/pinned_allocator.h>
#include <thrust/system_error.h>
#include <thrust/system/cuda/error.h>

template<class T>
class managed_allocator
{
  public:

    T* allocate( size_t n )
    {
      T* result = nullptr;

      cudaError_t error = cudaMallocManaged( 
          &result, n * sizeof(T), cudaMemAttachGlobal );

      if ( error != cudaSuccess )
      {
        throw thrust::system_error( error, thrust::cuda_category(), 
            "managed_allocator::allocate(): cudaMallocManaged" );
      }

      return result;
    }; /** end allocate() */

    void deallocate( T* ptr, size_t )
    {
      cudaError_t error = cudaFree( ptr );

      if ( error != cudaSuccess )
      {
        throw thrust::system_error( error, thrust::cuda_category(), 
            "managed_allocator::deallocate(): cudaFree" );
      }
    }; /** end deallocate() */
};
#endif // ifdef HMLP_USE_CUDA





/** debug flag */
#define DEBUG_DATA 1


using namespace std;




namespace hmlp
{

#ifdef HMLP_MIC_AVX512
/** use hbw::allocator for Intel Xeon Phi */
template<class T, class Allocator = hbw::allocator<T> >
#elif  HMLP_USE_CUDA
/** use pinned (page-lock) memory for NVIDIA GPUs */
template<class T, class Allocator = thrust::system::cuda::experimental::pinned_allocator<T> >
#else
/** use default stl allocator */
template<class T, class Allocator = std::allocator<T> >
#endif
class Data : public ReadWrite, public vector<T, Allocator>
#ifdef HMLP_USE_CUDA
/** inheritate the interface fot the host-device model. */
, public DeviceMemory<T>
#endif
{
  public:

    /** Default empty constructor. */
    Data() : vector<T, Allocator>(), m( 0 ), n( 0 ) {};

    /** Copy constructor for hmlp::Data. */
    Data( const Data<T>& other_data ) : vector<T, Allocator>( other_data )
    {
      this->m = other_data.row();
      this->n = other_data.col();
    }

    /** TODO: Copy constructor for std::vector. */
    Data( size_t m, size_t n, const vector<T>& other_vector ) 
      : vector<T, Allocator>( other_vector )
    {
      assert( other_vector.size() == m * n );
      this->m = m;
      this->n = n;
    };

    Data( size_t m, size_t n ) : vector<T, Allocator>( m * n )
    { 
      this->m = m;
      this->n = n;
    };

    Data( size_t m, size_t n, T initT ) : vector<T, Allocator>( m * n, initT )
    { 
      this->m = m;
      this->n = n;
    };

    Data( size_t m, size_t n, string &filename ) : vector<T, Allocator>( m * n )
    {
      this->m = m;
      this->n = n;
      this->read( m, n, filename );
    };

    void resize( size_t m, size_t n )
    { 
      this->m = m;
      this->n = n;
      vector<T, Allocator>::resize( m * n );
    };

    void resize( size_t m, size_t n, T initT )
    {
      this->m = m;
      this->n = n;
      vector<T, Allocator>::resize( m * n, initT );
    };

    void reserve( size_t m, size_t n ) 
    {
      vector<T, Allocator>::reserve( m * n );
    };

    void clear()
    {
      this->m = 0;
      this->n = 0;
      vector<T, Allocator>::clear();
    };

    void read( size_t m, size_t n, string &filename )
    {
      assert( this->m == m );
      assert( this->n == n );
      assert( this->size() == m * n );

      cout << filename << endl;

      ifstream file( filename.data(), ios::in|ios::binary|ios::ate );
      if ( file.is_open() )
      {
        auto size = file.tellg();
        assert( size == m * n * sizeof(T) );
        file.seekg( 0, ios::beg );
        file.read( (char*)this->data(), size );
        file.close();
      }
    };

		void write( std::string &filename )
		{
			ofstream myFile ( filename.data(), ios::out | ios::binary );
      myFile.write( (char*)(this->data()), this->size() * sizeof(T) );
		};

    template<int SKIP_ATTRIBUTES = 0, bool TRANS = false>
		void readmtx( size_t m, size_t n, string &filename )
		{
      assert( this->m == m );
      assert( this->n == n );
      assert( this->size() == m * n );

      cout << filename << endl;

      ifstream file( filename.data() );
			string line;
			if ( file.is_open() )
			{
				size_t j = 0;
				while ( getline( file, line ) )
				{
					if ( j == 0 ) printf( "%s\n", line.data() );


					if ( j % 1000 == 0 ) printf( "%4lu ", j ); fflush( stdout );
					if ( j >= n )
					{
						printf( "more data then execpted n %lu\n", n );
					}

          /** Replace all ',' and ';' with white space ' ' */
					replace( line.begin(), line.end(), ',', '\n' );
					replace( line.begin(), line.end(), ';', '\n' );

					istringstream iss( line );

					for ( size_t i = 0; i < m + SKIP_ATTRIBUTES; i ++ )
					{
						T tmp;
						if ( !( iss >> tmp ) )
						{
							printf( "line %lu does not have enough elements (only %lu)\n", j, i );
					    printf( "%s\n", line.data() );
							exit( 1 );
						}
						if ( i >= SKIP_ATTRIBUTES )
						{
							if ( TRANS ) (*this)[ j * m + i ] = tmp;
							else         (*this)[ i * n + j ] = tmp;
						}
					}
					j ++;
				}
				printf( "\nfinish readmatrix %s\n", filename.data() );
			}
		};



    tuple<size_t, size_t> shape()
    {
      return make_tuple( m, n );
    };

   
    T* rowdata( size_t i ) 
    {
      assert( i < m );
      return ( this->data() + i );
    };

    T* columndata( size_t j )
    {
      assert( j < n );
      return ( this->data() + j * m );
    };

		T getvalue( size_t i ) { return (*this)[ i ]; };

		void setvalue( size_t i, T v ) { (*this)[ i ] = v; };

		T getvalue( size_t i, size_t j ) { return (*this)( i, j ); };

		void setvalue( size_t i, size_t j, T v ) { (*this)( i, j ) = v; };

    /** ESSENTIAL: return number of coumns */
    size_t row() const noexcept { return m; };

    /** ESSENTIAL: return number of rows */
    size_t col() const noexcept { return n; };

    /** ESSENTIAL: return an element */
    T& operator()( size_t i, size_t j )       { return (*this)[ m * j + i ]; };
    T  operator()( size_t i, size_t j ) const { return (*this)[ m * j + i ]; };


    /** ESSENTIAL: return a submatrix */
    Data<T> operator()( const vector<size_t>& I, const vector<size_t>& J ) const
    {
      Data<T> KIJ( I.size(), J.size() );
      for ( int j = 0; j < J.size(); j ++ )
      {
        for ( int i = 0; i < I.size(); i ++ )
        {
          KIJ[ j * I.size() + i ] = (*this)[ m * J[ j ] + I[ i ] ];
        }
      }
      return KIJ;
    }; 

    /** ESSENTIAL: */
    pair<T, size_t> ImportantSample( size_t j )
    {
      size_t i = std::rand() % m;
      pair<T, size_t> sample( (*this)( i, j ), i );
      return sample; 
    };


    Data<T> operator()( const vector<size_t> &jmap )
    {
      Data<T> submatrix( m, jmap.size() );
      #pragma omp parallel for
      for ( int j = 0; j < jmap.size(); j ++ )
        for ( int i = 0; i < m; i ++ )
          submatrix[ j * m + i ] = (*this)[ m * jmap[ j ] + i ];
      return submatrix;
    };

    template<typename TINDEX>
    void GatherColumns( bool TRANS, vector<TINDEX> &jmap, Data<T> &submatrix )
    {
      if ( TRANS )
      {
        submatrix.resize( jmap.size(), m );
        for ( int j = 0; j < jmap.size(); j ++ )
          for ( int i = 0; i < m; i ++ )
            submatrix[ i * jmap.size() + j ] = (*this)[ m * jmap[ j ] + i ];
      }
      else
      {
        submatrix.resize( m, jmap.size() );
        for ( int j = 0; j < jmap.size(); j ++ )
          for ( int i = 0; i < m; i ++ )
            submatrix[ j * m + i ] = (*this)[ m * jmap[ j ] + i ];
      }
    }; 

    void setvalue( T value )
    {
      for ( auto it = this->begin(); it != this->end(); it ++ )
        (*it) = value;
    };

    template<bool SYMMETRIC = false>
    void rand( T a, T b )
    {
      default_random_engine generator;
      uniform_real_distribution<T> distribution( a, b );

      if ( SYMMETRIC ) assert( m == n );

      for ( std::size_t j = 0; j < n; j ++ )
      {
        for ( std::size_t i = 0; i < m; i ++ )
        {
          if ( SYMMETRIC )
          {
            if ( i > j )
              (*this)[ j * m + i ] = distribution( generator );
            else
              (*this)[ j * m + i ] = (*this)[ i * m + j ];
          }
          else
          {
            (*this)[ j * m + i ] = distribution( generator );
          }
        }
      }
    };

    template<bool SYMMETRIC = false>
    void rand() 
    { 
      rand<SYMMETRIC>( 0.0, 1.0 ); 
    };

    void randn( T mu, T sd )
    {
      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine generator( seed );
      std::normal_distribution<T> distribution( mu, sd );
      for ( std::size_t i = 0; i < m * n; i ++ )
      {
        (*this)[ i ] = distribution( generator );
      }
    };

    void randn() { randn( 0.0, 1.0 ); };

    template<bool USE_LOWRANK=true>
    void randspd( T a, T b )
    {
      std::default_random_engine generator;
      std::uniform_real_distribution<T> distribution( a, b );

      assert( m == n );

      if ( USE_LOWRANK )
      {
        hmlp::Data<T> X( ( std::rand() % n ) / 10 + 1, n );
        X.randn( a, b );
        xgemm
        (
          "T", "N",
          n, n, X.row(),
          1.0, X.data(), X.row(),
               X.data(), X.row(),
          0.0, this->data(), this->row()
        );
      }
      else /** diagonal dominating */
      {
        for ( std::size_t j = 0; j < n; j ++ )
        {
          for ( std::size_t i = 0; i < m; i ++ )
          {
            if ( i > j )
              (*this)[ j * m + i ] = distribution( generator );
            else
              (*this)[ j * m + i ] = (*this)[ i * m + j ];

            /** Make sure diagonal dominated */
            (*this)[ j * m + j ] += std::abs( (*this)[ j * m + i ] );
          }
        }
      }
    };

    void randspd() 
    { 
      randspd<true>( 0.0, 1.0 ); 
    };

    bool HasIllegalValue()
    {
      for ( auto it = this->begin(); it != this->end(); it ++ )
      {
        if ( std::isinf( *it ) ) return true;
        if ( std::isnan( *it ) ) return true;
      }
      return false;
    };

    void Print()
    {
      printf( "Data in %lu * %lu\n", m, n );
      if ( m < 11 && n < 7 )
      {
        hmlp_printmatrix( m, n, this->data(), m );
      }
      else
      {
        hmlp_printmatrix( 10, 6, this->data(), m );
      }
    };

    void WriteFile( char *name )
    {
      FILE * pFile;
      pFile = fopen ( name, "w" );
      fprintf( pFile, "%s=[\n", name );
      for ( size_t i = 0; i < m; i ++ )
      {
        for ( size_t j = 0; j < n; j ++ )
        {
          fprintf( pFile, "%lf,", (*this)( i, j ) );
        }
        fprintf( pFile, ";\n" );
      }
      fprintf( pFile, "];\n" );
      fclose( pFile );
    };

    template<typename TINDEX>
    double flops( TINDEX na, TINDEX nb ) { return 0.0; };

#ifdef HMLP_USE_CUDA

    void CacheD( hmlp::Device *dev )
    {
      DeviceMemory<T>::CacheD( dev, this->size() * sizeof(T) );
    };

    void AllocateD( hmlp::Device *dev )
    {
      double beg = omp_get_wtime();
      DeviceMemory<T>::AllocateD( dev, m * n * sizeof(T) );
      double alloc_time = omp_get_wtime() - beg;
      printf( "AllocateD %5.3lf\n", alloc_time );
    };

    void FreeD( Device *dev )
    {
      DeviceMemory<T>::FreeD( dev, this->size() * sizeof(T) );
    };

    void PrefetchH2D( Device *dev, int stream_id )
    {
      double beg = omp_get_wtime();
      DeviceMemory<T>::PrefetchH2D
        ( dev, stream_id, m * n * sizeof(T), this->data() );
      double alloc_time = omp_get_wtime() - beg;
      //printf( "PrefetchH2D %5.3lf\n", alloc_time );
    };

    void PrefetchD2H( Device *dev, int stream_id )
    {
      DeviceMemory<T>::PrefetchD2H
        ( dev, stream_id, m * n * sizeof(T), this->data() );
    };

    void WaitPrefetch( Device *dev, int stream_id )
    {
      DeviceMemory<T>::Wait( dev, stream_id );
    };

    void FetchH2D( Device *dev )
    {
      DeviceMemory<T>::FetchH2D( dev, m * n * sizeof(T), this->data() );
    };

    void FetchD2H( Device *dev )
    {
      DeviceMemory<T>::FetchD2H( dev, m * n * sizeof(T), this->data() );
    };
#endif

  private:

    size_t m = 0;

    size_t n = 0;

}; /** end class Data */






template<typename T, class Allocator = std::allocator<T> >
class SparseData : public ReadWrite
{
  public:

    /** (Default) constructor. */
    SparseData( size_t m = 0, size_t n = 0, size_t nnz = 0, bool issymmetric = true )
    {
      Resize( m, n, nnz, issymmetric );
    };

    /** Adjust the storage size. */
    void Resize( size_t m, size_t n, size_t nnz, bool issymmetric )
    {
      this->m = m;
      this->n = n;
      this->nnz = nnz;
      this->val.resize( nnz, 0.0 );
      this->row_ind.resize( nnz, 0 );
      this->col_ptr.resize( n + 1, 0 );
      this->issymmetric = issymmetric;
    }; /** end Resize() */

    /** Construct from three arrays: val[ nnz ],row_ind[ nnz ], and col_ptr[ n + 1 ]. */
    void fromCSC( size_t m, size_t n, size_t nnz, bool issymmetric,
        const T *val, const size_t *row_ind, const size_t *col_ptr ) 
    {
      Resize( m, n, nnz, issymmetric );
      for ( size_t i = 0; i < nnz; i ++ )
      {
        this->val[ i ] = val[ i ];
        this->row_ind[ i ] = row_ind[ i ];
      }
      for ( size_t j = 0; j < n + 1; j ++ )
      {
        this->col_ptr[ j ] = col_ptr[ j ];
      }
    }; /** end fromCSC()*/

    /** Retrive an element K( i, j ).  */
    T operator () ( size_t i, size_t j ) const
    {
      if ( issymmetric && i < j ) std::swap( i, j );
      auto row_beg = col_ptr[ j ];
      auto row_end = col_ptr[ j + 1 ];
      /** Early return if there is no nonzero entry in this column. */
      if ( row_beg == row_end ) return 0;
      /** Search (BST) for row indices. */
      auto lower = find( row_ind.begin() + row_beg, row_ind.begin() + row_end - 1, i );
      //if ( lower != row_ind.end() ) printf( "lower %lu, i %lu, j %lu, row_beg %lu, row_end %lu\n", 
      //    *lower, i, j, row_beg, row_end ); fflush( stdout );
      /** If the lower bound matches, then return the value. */
      if ( *lower == i ) return val[ distance( row_ind.begin(), lower ) ];
      /** Otherwise, return 0. */
      return 0;
    }; /** end operator () */

    /** Retrive a subblock K( I, J ).*/
    Data<T> operator()( const vector<size_t> &I, const vector<size_t> &J ) const
    {
      Data<T> KIJ( I.size(), J.size() );
      /** Evaluate Kij element by element. */
      for ( size_t j = 0; j < J.size(); j ++ )
        for ( size_t i = 0; i < I.size(); i ++ )
          KIJ( i, j ) = (*this)( I[ i ], J[ j ] );
      /** Return submatrix KIJ. */
      return KIJ;
    }; /** end operator () */ 

    size_t ColPtr( size_t j ) { return col_ptr[ j ]; };

    size_t RowInd( size_t offset ) { return row_ind[ offset ]; };

    T Value( size_t offset ) { return val[ offset ]; };

    pair<T, size_t> ImportantSample( size_t j )
    {
      size_t offset = col_ptr[ j ] + rand() % ( col_ptr[ j + 1 ] - col_ptr[ j ] );
      pair<T, size_t> sample( val[ offset ], row_ind[ offset ] );
      return sample; 
    };

    void Print()
    {
      for ( size_t j = 0; j < n; j ++ ) printf( "%8lu ", j );
      printf( "\n" );
      for ( size_t i = 0; i < m; i ++ )
      {
        for ( size_t j = 0; j < n; j ++ )
        {
          printf( "% 3.1E ", (*this)( i, j ) );
        }
        printf( "\n" );
      }
    }; // end Print()


    /** 
     *  @brief Read matrix market format (ijv) format. Only lower triangular
     *         part is stored
     */ 
    template<bool LOWERTRIANGULAR, bool ISZEROBASE, bool IJONLY = false>
    void readmtx( string &filename )
    {
      size_t m_mtx, n_mtx, nnz_mtx;

      vector<deque<size_t>> full_row_ind( n );
      vector<deque<T>> full_val( n );

      // Read all tuples.
      printf( "%s ", filename.data() ); fflush( stdout );
      ifstream file( filename.data() );
      string line;
      if ( file.is_open() )
      {
        size_t nnz_count = 0;

        while ( std::getline( file, line ) )
        {
          if ( line.size() )
          {
            if ( line[ 0 ] != '%' )
            {
              std::istringstream iss( line );
              iss >> m_mtx >> n_mtx >> nnz_mtx;
              assert( this->m == m_mtx );
              assert( this->n == n_mtx );
              assert( this->nnz == nnz_mtx );
              break;
            }
          }
        }

        while ( std::getline( file, line ) )
        {
          if ( nnz_count % ( nnz / 10 ) == 0 )
          {
            printf( "%lu%% ", ( nnz_count * 100 ) / nnz ); fflush( stdout );
          }

          std::istringstream iss( line );

					size_t i, j;
          T v;

          if ( IJONLY )
          {
            if ( !( iss >> i >> j ) )
            {
              printf( "line %lu has illegle format\n", nnz_count );
              break;
            }
            v = 1;
          }
          else
          {
            if ( !( iss >> i >> j >> v ) )
            {
              printf( "line %lu has illegle format\n", nnz_count );
              break;
            }
          }

          if ( !ISZEROBASE )
          {
            i -= 1;
            j -= 1;
          }

          if ( v != 0.0 )
          {
            full_row_ind[ j ].push_back( i );
            full_val[ j ].push_back( v );

            if ( LOWERTRIANGULAR && i > j  )
            {
              full_row_ind[ i ].push_back( j );
              full_val[ i ].push_back( v );
            }
          }
          nnz_count ++;
        }
        assert( nnz_count == nnz );
      }
      printf( "Done.\n" ); fflush( stdout );
      // Close the file.
      file.close();

      //printf( "Here nnz %lu\n", nnz );

      // Recount nnz for the full storage.
      size_t full_nnz = 0;
      for ( size_t j = 0; j < n; j ++ )
      {
        col_ptr[ j ] = full_nnz;
        full_nnz += full_row_ind[ j ].size();
      }
      nnz = full_nnz;
      col_ptr[ n ] = full_nnz;
      row_ind.resize( full_nnz );
      val.resize( full_nnz );

      //printf( "Here nnz %lu\n", nnz );

      //full_nnz = 0;
      //for ( size_t j = 0; j < n; j ++ )
      //{
      //  for ( size_t i = 0; i < full_row_ind[ j ].size(); i ++ )
      //  {
      //    row_ind[ full_nnz ] = full_row_ind[ j ][ i ];
      //    val[ full_nnz ] = full_val[ j ][ i ];
      //    full_nnz ++;
      //  }
      //}

      //printf( "Close the file. Reformat.\n" );

      #pragma omp parallel for
      for ( size_t j = 0; j < n; j ++ )
      {
        for ( size_t i = 0; i < full_row_ind[ j ].size(); i ++ )
        {
          row_ind[ col_ptr[ j ] + i ] = full_row_ind[ j ][ i ];
          val[ col_ptr[ j ] + i ] = full_val[ j ][ i ];
        }
      }

      printf( "finish readmatrix %s\n", filename.data() ); fflush( stdout );
    };


    size_t row() { return m; };

    size_t col() { return n; };
    
    template<typename TINDEX>
    double flops( TINDEX na, TINDEX nb ) { return 0.0; };

  private:

    size_t m = 0;

    size_t n = 0;

    size_t nnz = 0;

    bool issymmetric = false;

    vector<T, Allocator> val;

    vector<size_t> row_ind;
   
    vector<size_t> col_ptr;

}; /** end class CSC */



#ifdef HMLP_MIC_AVX512
template<class T, class Allocator = hbw::allocator<T> >
#else
template<class T, class Allocator = std::allocator<T> >
#endif
class OOCData : public ReadWrite
{
  public:

    OOCData() {};

    OOCData( size_t m, size_t n, string filename ) { Set( m, n, filename ); };

    ~OOCData()
    {
      /** Unmap */
      int rc = munmap( mmappedData, m * n * sizeof(T) );
      assert( rc == 0 );
      close( fd );
      printf( "finish readmatrix %s\n", filename.data() );
    };

    void Set( size_t m, size_t n, string filename )
    {
      this->m = m;
      this->n = n;
      this->filename = filename;
      /** Open the file */
      fd = open( filename.data(), O_RDONLY, 0 ); 
      assert( fd != -1 );
#ifdef __APPLE__
      mmappedData = (T*)mmap( NULL, m * n * sizeof(T), 
          PROT_READ, MAP_PRIVATE, fd, 0 );
#else /** Assume Linux */
      mmappedData = (T*)mmap( NULL, m * n * sizeof(T), 
          PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0 );
#endif
      assert( mmappedData != MAP_FAILED );
      cout << filename << endl;
    };

    //template<typename TINDEX>
    T operator()( size_t i, size_t j ) const 
    {
      assert( i < m && j < n );
      return mmappedData[ j * m + i ];
    };

    //template<typename TINDEX>
    Data<T> operator()( const vector<size_t>& I, const vector<size_t>& J ) const 
    {
      Data<T> KIJ( I.size(), J.size() );
      for ( int j = 0; j < J.size(); j ++ )
        for ( int i = 0; i < I.size(); i ++ )
          KIJ[ j * I.size() + i ] = (*this)( I[ i ], J[ j ] );
      return KIJ;
    }; 

    template<typename TINDEX>
    pair<T, TINDEX> ImportantSample( TINDEX j )
    {
      TINDEX i = std::rand() % m;
      pair<T, TINDEX> sample( (*this)( i, j ), i );
      return sample; 
    };

    size_t row() const noexcept { return m; };

    size_t col() const noexcept { return n; };

    template<typename TINDEX>
    double flops( TINDEX na, TINDEX nb ) { return 0.0; };

  private:

    size_t m = 0;

    size_t n = 0;

    string filename;

    /** Use mmap */
    T *mmappedData = NULL;

    int fd = -1;

}; /** end class OOCData */







}; /** end namespace hmlp */

#endif /** define DATA_HPP */
