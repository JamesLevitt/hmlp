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


#ifndef MPITREE_HPP
#define MPITREE_HPP

/** Inherit most of the classes from shared-memory GOFMM. */
#include <tree.hpp>
/** Use HMLP related support. */
#include <hmlp_mpi.hpp>
/** Use distributed matrices inspired by the Elemental notation. */
#include <DistData.hpp>
/** Use STL and HMLP namespaces. */
using namespace std;
using namespace hmlp;


namespace hmlp
{
namespace mpitree
{



///**
// *  @brief This is the default ball tree splitter. Given coordinates,
// *         compute the direction from the two most far away points.
// *         Project all points to this line and split into two groups
// *         using a median select.
// *
// *  @para
// *
// *  @TODO  Need to explit the parallelism.
// */ 
//template<int N_SPLIT, typename T>
//struct centersplit
//{
//  // closure
//  Data<T> *Coordinate;
//
//  inline vector<vector<size_t> > operator()
//  ( 
//    vector<size_t>& gids
//  ) const 
//  {
//    assert( N_SPLIT == 2 );
//
//    Data<T> &X = *Coordinate;
//    size_t d = X.row();
//    size_t n = gids.size();
//
//    T rcx0 = 0.0, rx01 = 0.0;
//    size_t x0, x1;
//    vector<vector<size_t> > split( N_SPLIT );
//
//
//    vector<T> centroid = combinatorics::Mean( d, n, X, gids );
//    vector<T> direction( d );
//    vector<T> projection( n, 0.0 );
//
//    //printf( "After Mean\n" );
//
//    // Compute the farest x0 point from the centroid
//    for ( int i = 0; i < n; i ++ )
//    {
//      T rcx = 0.0;
//      for ( int p = 0; p < d; p ++ )
//      {
//        T tmp = X[ gids[ i ] * d + p ] - centroid[ p ];
//        rcx += tmp * tmp;
//      }
//      //printf( "\n" );
//      if ( rcx > rcx0 ) 
//      {
//        rcx0 = rcx;
//        x0 = i;
//      }
//    }
//
//
//    // Compute the farest point x1 from x0
//    for ( int i = 0; i < n; i ++ )
//    {
//      T rxx = 0.0;
//      for ( int p = 0; p < d; p ++ )
//      {
//        T tmp = X[ gids[ i ] * d + p ] - X[ gids[ x0 ] * d + p ];
//        rxx += tmp * tmp;
//      }
//      if ( rxx > rx01 )
//      {
//        rx01 = rxx;
//        x1 = i;
//      }
//    }
//
//    // Compute direction
//    for ( int p = 0; p < d; p ++ )
//    {
//      direction[ p ] = X[ gids[ x1 ] * d + p ] - X[ gids[ x0 ] * d + p ];
//    }
//
//    // Compute projection
//    projection.resize( n, 0.0 );
//    for ( int i = 0; i < n; i ++ )
//      for ( int p = 0; p < d; p ++ )
//        projection[ i ] += X[ gids[ i ] * d + p ] * direction[ p ];
//
//    /** Parallel median search */
//    T median;
//    
//    if ( 1 )
//    {
//      median = hmlp::combinatorics::Select( n, n / 2, projection );
//    }
//    else
//    {
//      auto proj_copy = projection;
//      std::sort( proj_copy.begin(), proj_copy.end() );
//      median = proj_copy[ n / 2 ];
//    }
//
//    split[ 0 ].reserve( n / 2 + 1 );
//    split[ 1 ].reserve( n / 2 + 1 );
//
//
//    /** TODO: Can be parallelized */
//    std::vector<std::size_t> middle;
//    for ( size_t i = 0; i < n; i ++ )
//    {
//      if      ( projection[ i ] < median ) split[ 0 ].push_back( i );
//      else if ( projection[ i ] > median ) split[ 1 ].push_back( i );
//      else                                 middle.push_back( i );
//    }
//
//    for ( size_t i = 0; i < middle.size(); i ++ )
//    {
//      if ( split[ 0 ].size() <= split[ 1 ].size() ) split[ 0 ].push_back( middle[ i ] );
//      else                                          split[ 1 ].push_back( middle[ i ] );
//    }
//
//
//    return split;
//  };
//
//
//  inline std::vector<std::vector<size_t> > operator()
//  ( 
//    std::vector<size_t>& gids,
//    hmlp::mpi::Comm comm
//  ) const 
//  {
//    std::vector<std::vector<size_t> > split( N_SPLIT );
//
//    return split;
//  };
//
//};
//
//
//
//
//
//template<int N_SPLIT, typename T>
//struct randomsplit
//{
//  Data<T> *Coordinate = NULL;
//
//  inline vector<vector<size_t> > operator() ( vector<size_t>& gids ) const 
//  {
//    vector<vector<size_t> > split( N_SPLIT );
//    return split;
//  };
//
//  inline vector<vector<size_t> > operator() ( vector<size_t>& gids, mpi::Comm comm ) const 
//  {
//    vector<vector<size_t> > split( N_SPLIT );
//    return split;
//  };
//};
//





template<typename NODE>
class DistSplitTask : public Task
{
  public:

    NODE *arg = NULL;

    void Set( NODE *user_arg )
    {
      arg = user_arg;
      name = string( "DistSplit" );
      label = to_string( arg->treelist_id );

      double flops = 6.0 * arg->n;
      double  mops = 6.0 * arg->n;

      /** Setup the event */
      event.Set( label + name, flops, mops );
      /** Asuume computation bound */
      cost = mops / 1E+9;
      /** "HIGH" priority */
      priority = true;
    };


    void DependencyAnalysis()
    {
      arg->DependencyAnalysis( R, this );

      if ( !arg->isleaf )
      {
        if ( arg->GetCommSize() > 1 )
        {
					assert( arg->child );
          arg->child->DependencyAnalysis( RW, this );
        }
        else
        {
					assert( arg->lchild && arg->rchild );
          arg->lchild->DependencyAnalysis( RW, this );
          arg->rchild->DependencyAnalysis( RW, this );
        }
      }
      this->TryEnqueue();
		};

    void Execute( Worker* user_worker ) { arg->Split(); };

}; /** end class DistSplitTask */




/**
 *  @brief Data and setup that are shared with all nodes.
 */ 
template<typename SPLITTER, typename DATATYPE>
class Setup
{
  public:

    typedef DATATYPE T;

    Setup() {};

    ~Setup() {};




    /**
     *  @brief Check if this node contain any query using morton.
		 *         Notice that queries[] contains gids; thus, morton[]
		 *         needs to be accessed using gids.
     *
     */ 
		vector<size_t> ContainAny( vector<size_t> &queries, size_t target )
    {
			vector<size_t> validation( queries.size(), 0 );

      if ( !morton.size() )
      {
        printf( "Morton id was not initialized.\n" );
        exit( 1 );
      }

      for ( size_t i = 0; i < queries.size(); i ++ )
      {
				/** notice that setup->morton only contains local morton ids */
        //auto it = this->setup->morton.find( queries[ i ] );

				//if ( it != this->setup->morton.end() )
				//{
        //  if ( tree::IsMyParent( *it, this->morton ) ) validation[ i ] = 1;
				//}


       if ( MortonHelper::IsMyParent( morton[ queries[ i ] ], target ) ) 
				 validation[ i ] = 1;

      }
      return validation;

    }; /** end ContainAny() */




    /** maximum leaf node size */
    size_t m;
    
    /** by default we use 4 bits = 0-15 levels */
    size_t max_depth = 15;

    /** coordinates (accessed with gids) */
    //DistData<STAR, CBLK, T> *X_cblk = NULL;
    //DistData<STAR, CIDS, T> *X      = NULL;

    /** neighbors<distance, gid> (accessed with gids) */
    DistData<STAR, CBLK, pair<T, size_t>> *NN_cblk = NULL;
    DistData<STAR, CIDS, pair<T, size_t>> *NN      = NULL;

    /** morton ids */
    vector<size_t> morton;

    /** tree splitter */
    SPLITTER splitter;

}; /** end class Setup */



template<typename NODE>
class DistIndexPermuteTask : public Task
{
  public:

    NODE *arg = NULL;

    void Set( NODE *user_arg )
    {
      name = std::string( "Permutation" );
      arg = user_arg;
      // Need an accurate cost model.
      cost = 1.0;
    };

    void DependencyAnalysis() { arg->DependOnChildren( this ); };
    //{
    //  arg->DependencyAnalysis( hmlp::ReadWriteType::RW, this );
    //  if ( !arg->isleaf && !arg->child )
    //  {
    //    arg->lchild->DependencyAnalysis( hmlp::ReadWriteType::R, this );
    //    arg->rchild->DependencyAnalysis( hmlp::ReadWriteType::R, this );
    //  }
    //  this->TryEnqueue();
    //};


    void Execute( Worker* user_worker )
    {
      if ( !arg->isleaf && !arg->child )
      {
        auto &gids = arg->gids; 
        auto &lgids = arg->lchild->gids;
        auto &rgids = arg->rchild->gids;
        gids = lgids;
        gids.insert( gids.end(), rgids.begin(), rgids.end() );
      }
    };

}; /** end class IndexPermuteTask */
















/**
 *
 */ 
//template<typename SETUP, int N_CHILDREN, typename NODEDATA>
template<typename SETUP, typename NODEDATA>
class Node : public tree::Node<SETUP, NODEDATA>
{
  public:

    /** Deduce data type from SETUP. */
    typedef typename SETUP::T T;

    static const int N_CHILDREN = 2;

    /** Inherit all parameters from tree::Node */


    /** (Default) constructor for inner nodes (gids and n unassigned) */
    Node( SETUP *setup, size_t n, size_t l, 
        Node *parent,
        unordered_map<size_t, tree::Node<SETUP, NODEDATA>*> *morton2node,
        Lock *treelock, mpi::Comm comm ) 
      : tree::Node<SETUP, NODEDATA>( setup, n, l, 
          parent, morton2node, treelock ) 
    {
      /** Local communicator */
      this->comm = comm;
      /** Get MPI size and rank. */
      mpi::Comm_size( comm, &size );
      mpi::Comm_rank( comm, &rank );
    };

    /** (Default) constructor for root. */
    Node( SETUP *setup, size_t n, size_t l, vector<size_t> &gids, 
        Node *parent, 
        unordered_map<size_t, tree::Node<SETUP, NODEDATA>*> *morton2node,
        Lock *treelock, mpi::Comm comm ) 
      : Node<SETUP, NODEDATA>( setup, n, l, parent, 
          morton2node, treelock, comm ) 
    {
      /** Notice that "gids.size() < n". */
      this->gids = gids;
    };

    /** (Default) constructor for LET nodes. */
    Node( size_t morton ) : tree::Node<SETUP, NODEDATA>( morton )
    { 
    };


    //void SetupChild( class Node *child )
    //{
    //  this->kids[ 0 ] = child;
    //  this->child = child;
    //};



    /** */ 
    void Split()
    {
      /** Reduce to get the total size of gids. */
      int num_points_total = 0;
      int num_points_owned = (this->gids).size();
      /** n = sum( num_points_owned ) over all MPI processes in comm. */
      mpi::Allreduce( &num_points_owned, &num_points_total, 1, MPI_SUM, comm );
      this->n = num_points_total;

      if ( child )
      {
        /** The local communicator of this node contains at least 2 processes. */
        assert( size > 1 );

        /** Invoke distributed splitter. */
        auto split = this->setup->splitter( this->gids, comm );

        /** Get partner MPI rank. */
        int partner_rank = 0;
        int sent_size = 0; 
        int recv_size = 0;
        vector<size_t> &kept_gids = child->gids;
        vector<int>     sent_gids;
        vector<int>     recv_gids;

        if ( rank < size / 2 ) 
        {
          /** left child */
          partner_rank = rank + size / 2;
          /** MPI ranks 0:size/2-1 keep split[ 0 ] */
          kept_gids.resize( split[ 0 ].size() );
          for ( size_t i = 0; i < kept_gids.size(); i ++ )
            kept_gids[ i ] = this->gids[ split[ 0 ][ i ] ];
          /** MPI ranks 0:size/2-1 send split[ 1 ] */
          sent_gids.resize( split[ 1 ].size() );
          sent_size = sent_gids.size();
          for ( size_t i = 0; i < sent_gids.size(); i ++ )
            sent_gids[ i ] = this->gids[ split[ 1 ][ i ] ];
        }
        else       
        {
          /** right child */
          partner_rank = rank - size / 2;
          /** MPI ranks size/2:size-1 keep split[ 1 ] */
          kept_gids.resize( split[ 1 ].size() );
          for ( size_t i = 0; i < kept_gids.size(); i ++ )
            kept_gids[ i ] = this->gids[ split[ 1 ][ i ] ];
          /** MPI ranks size/2:size-1 send split[ 0 ] */
          sent_gids.resize( split[ 0 ].size() );
          sent_size = sent_gids.size();
          for ( size_t i = 0; i < sent_gids.size(); i ++ )
            sent_gids[ i ] = this->gids[ split[ 0 ][ i ] ];
        }
        assert( partner_rank >= 0 );





        ///** Exchange recv_gids.size(). */
        //mpi::Sendrecv( &sent_size, 1, partner_rank, 10,
        //               &recv_size, 1, partner_rank, 10, comm, &status );

        //printf( "rank %d kept_size %lu sent_size %d recv_size %d\n", 
        //    rank, kept_gids.size(), sent_size, recv_size ); fflush( stdout );

        ///** resize recv_gids */
        //recv_gids.resize( recv_size );

        ///** Exchange recv_gids.size() */
        //mpi::Sendrecv( 
        //    sent_gids.data(), sent_size, MPI_INT, partner_rank, 20,
        //    recv_gids.data(), recv_size, MPI_INT, partner_rank, 20,
        //    comm, &status );

        /** Exchange gids with my partner. */
        mpi::ExchangeVector( sent_gids, partner_rank, 20,
                             recv_gids, partner_rank, 20, comm, &status );
        /** kept_gids += recv_gids. */
        for ( auto it : recv_gids ) kept_gids.push_back( it );
        //kept_gids.reserve( kept_gids.size() + recv_gids.size() );
        //for ( size_t i = 0; i < recv_gids.size(); i ++ )
        //  kept_gids.push_back( recv_gids[ i ] );
        

      }
			else
			{
        tree::Node<SETUP, NODEDATA>::Split();
		  }
      /** Synchronize within local communicator */
      mpi::Barrier( comm );
    }; /** end Split() */


    /** @brief Support dependency analysis. */
    void DependOnChildren( Task *task )
    {
      this->DependencyAnalysis( RW, task );
      if ( size < 2 )
      {
        if ( this->lchild ) this->lchild->DependencyAnalysis( R, task );
        if ( this->rchild ) this->rchild->DependencyAnalysis( R, task );
      }
      else
      {
        if ( child ) child->DependencyAnalysis( R, task );
      }
      /** Try to enqueue if there is no dependency. */
      task->TryEnqueue();
    }; /** end DependOnChildren() */

    /** @brief */
    void DependOnParent( Task *task )
    {
      this->DependencyAnalysis( R, task );
      if ( size < 2 )
      {
        if ( this->lchild ) this->lchild->DependencyAnalysis( RW, task );
        if ( this->rchild ) this->rchild->DependencyAnalysis( RW, task );
      }
      else
      {
        if ( child ) child->DependencyAnalysis( RW, task );
      }
      /** Try to enqueue if there is no dependency. */
      task->TryEnqueue();
    }; /** end DependOnParent() */


    /** Preserve for debugging. */
    void Print() {};

    /** Return local MPI communicator. */
    mpi::Comm GetComm() { return comm; };
    /** Return local MPI size. */
    int GetCommSize() { return size; };
    /** Return local MPI rank. */
    int GetCommRank() { return rank; };

    /** Distributed tree nodes only have one child. */
    Node *child = NULL;

  private:

    /** Initialize with all processes. */
    mpi::Comm comm = MPI_COMM_WORLD;
    /** MPI status. */
    mpi::Status status;
    /** Subcommunicator size. */
    int size = 1;
    /** Subcommunicator rank. */
    int rank = 0;

}; /** end class Node */










/**
 *  @brief This distributed tree inherits the shared memory tree
 *         with some additional MPI data structure and function call.
 */ 
template<class SETUP, class NODEDATA>
class Tree : public tree::Tree<SETUP, NODEDATA>,
             public mpi::MPIObject
{
  public:

    typedef typename SETUP::T T;

    /** 
     *  Inherit parameters n, m, and depth; local treelists and morton2node map.
     *
     *  Explanation for the morton2node map in the distributed tree:
     *
     *  morton2node has type map<size_t, tree::Node>, but it actually contains
     *  "three" different kinds of tree nodes.
     *
     *  1. Local tree nodes (exactly in type tree::Node)
     *
     *  2. Distributed tree nodes (in type mpitree::Node)
     *
     *  3. Local essential nodes (in type tree::Node with essential data)
     *
     */

    /** 
     *  Define local tree node type as NODE. Notice that all pointers in the
     *  interaction lists and morton2node map will be in this type.
     */
    typedef tree::Node<SETUP, NODEDATA> NODE;

    /** Define distributed tree node type as MPINODE.  */
    typedef Node<SETUP, NODEDATA> MPINODE;

    /** 
     *  Distribued tree nodes in the top-down order. Notice thay
     *  mpitreelist.back() is the root of the local tree.
     *
     *  i.e. mpitrelist.back() == treelist.front();
     */
    vector<MPINODE*> mpitreelists;

    
    /** (Default) Tree constructor */ 
    Tree( mpi::Comm comm ) : tree::Tree<SETUP, NODEDATA>::Tree(),
                             mpi::MPIObject( comm )
    {
      //this->comm = comm;
			/** Get size and rank */
      //mpi::Comm_size( comm, &size );
      //mpi::Comm_rank( comm, &rank );
      /** Create a ReadWrite object per rank */
      //NearRecvFrom.resize( size );
      NearRecvFrom.resize( this->GetCommSize() );
      //FarRecvFrom.resize( size );
      FarRecvFrom.resize( this->GetCommSize() );
    };

    /** (Default) Tree destructor.  */
    ~Tree()
    {
      //printf( "~Tree() distributed, mpitreelists.size() %lu\n",
      //    mpitreelists.size() ); fflush( stdout );
      /** 
       *  we do not free the last tree node, it will be deleted by 
       *  hmlp::tree::Tree()::~Tree() 
       */
      if ( mpitreelists.size() )
      {
        for ( size_t i = 0; i < mpitreelists.size() - 1; i ++ )
          if ( mpitreelists[ i ] ) delete mpitreelists[ i ];
        mpitreelists.clear();
      }
      //printf( "end ~Tree() distributed\n" ); fflush( stdout );
    };


    /**
     *  @brief free all tree nodes including local tree nodes,
     *         distributed tree nodes and let nodes
     */ 
    void CleanUp()
    {
      /** Free all local tree nodes */
      if ( this->treelist.size() )
      {
        for ( size_t i = 0; i < this->treelist.size(); i ++ )
          if ( this->treelist[ i ] ) delete this->treelist[ i ];
      }
      this->treelist.clear();

      /** Free all distributed tree nodes */
      if ( mpitreelists.size() )
      {
        for ( size_t i = 0; i < mpitreelists.size() - 1; i ++ )
          if ( mpitreelists[ i ] ) delete mpitreelists[ i ];
      }
      mpitreelists.clear();

    }; /** end CleanUp() */


    /** @breif Allocate all distributed tree nodse. */
    void AllocateNodes( vector<size_t> &gids )
    {
      /** Decide the depth of the distributed tree according to mpi size. */
      //auto mycomm  = comm;
      auto mycomm  = this->GetComm();
      //int mysize  = size;
      int mysize  = this->GetCommSize();
      //int myrank  = rank;
      int myrank  = this->GetCommRank();
      int mycolor = 0;
      size_t mylevel = 0;

      /** Allocate root( setup, n = 0, l = 0, parent = NULL ). */
      auto *root = new MPINODE( &(this->setup), 
          this->n, mylevel, gids, NULL, 
          &(this->morton2node), &(this->lock), mycomm );

      /** Push root to the mpi treelist. */
      mpitreelists.push_back( root );

      /** Recursively spliiting the communicator. */
      while ( mysize > 1 )
      {
        mpi::Comm childcomm;

        /** Increase level. */
        mylevel += 1;
        /** Left color = 0, right color = 1. */
        mycolor = ( myrank < mysize / 2 ) ? 0 : 1;
        /** Split and assign the subcommunicators for children. */
        ierr = mpi::Comm_split( mycomm, mycolor, myrank, &(childcomm) );
        /** Update mycomm, mysize, and myrank to proceed to the next iteration. */
        mycomm = childcomm;
        mpi::Comm_size( mycomm, &mysize );
        mpi::Comm_rank( mycomm, &myrank );

        /** Create the child node. */
        auto *parent = mpitreelists.back();
        auto *child  = new MPINODE( &(this->setup), 
            (size_t)0, mylevel, parent, 
            &(this->morton2node), &(this->lock), mycomm );

        /** Create the sibling in type NODE but not MPINODE. */
        child->sibling = new NODE( (size_t)0 ); // Node morton is computed later.
        /** Setup parent's children */
        //parent->SetupChild( child );
        parent->kids[ 0 ] = child;
        parent->child = child;
        /** Push to the mpi treelist */
        mpitreelists.push_back( child );
      }
      /** Global synchronization. */
      this->Barrier();

			/** Allocate local tree nodes. */
      auto *local_tree_root = mpitreelists.back();
      tree::Tree<SETUP, NODEDATA>::AllocateNodes( local_tree_root );

    }; /** end AllocateNodes() */




    vector<size_t> GetPermutation()
    {
      vector<size_t> perm_loc, perm_glb;
      perm_loc = tree::Tree<SETUP, NODEDATA>::GetPermutation();
      mpi::GatherVector( perm_loc, perm_glb, 0, this->GetComm() );

      //if ( rank == 0 )
      //{
      //  /** Sanity check using an 0:N-1 table. */
      //  vector<bool> Table( this->n, false );
      //  for ( size_t i = 0; i < perm_glb.size(); i ++ )
      //    Table[ perm_glb[ i ] ] = true;
      //  for ( size_t i = 0; i < Table.size(); i ++ ) assert( Table[ i ] );
      //}

      return perm_glb;
    }; /** end GetTreePermutation() */





    /** Perform approximate kappa neighbor search. */
    template<typename KNNTASK>
    DistData<STAR, CBLK, pair<T, size_t>>
    AllNearestNeighbor( size_t n_tree, size_t n, size_t k,
      pair<T, size_t> initNN, KNNTASK &dummy )
    {
      mpi::PrintProgress( "[BEG] NeighborSearch ...", this->GetComm() );

      /** Get the problem size from setup->K->row(). */
      this->n = n;
      /** k-by-N, column major. */
      DistData<STAR, CBLK, pair<T, size_t>> NN( k, n, initNN, this->GetComm() );
      /** Use leaf size = 4 * k.  */
      this->setup.m = 4 * k;
      if ( this->setup.m < 512 ) this->setup.m = 512;
      this->m = this->setup.m;


      ///** Local problem size (assuming Round-Robin) */
      ////num_points_owned = ( n - 1 ) / size + 1;
      //num_points_owned = ( n - 1 ) / this->GetCommSize() + 1;

      ///** Edge case */
      //if ( n % this->GetCommSize() )
      //{
      //  //if ( rank >= ( n % size ) ) num_points_owned -= 1;
      //  if ( this->GetCommRank() >= ( n % this->GetCommSize() ) ) 
      //    num_points_owned -= 1;
      //}

      /** Local problem size (assuming Round-Robin) */
      num_points_owned = n / this->GetCommSize();
      /** Edge case */
      if ( this->GetCommRank() < ( n % this->GetCommSize() ) ) 
         num_points_owned += 1;



      /** Initial gids distribution (asssuming Round-Robin) */
      vector<size_t> gids( num_points_owned, 0 );
      for ( size_t i = 0; i < num_points_owned; i ++ )
        //gids[ i ] = i * size + rank;
        gids[ i ] = i * this->GetCommSize() + this->GetCommRank();

      /** Allocate distributed tree nodes in advance. */
      AllocateNodes( gids );


      /** Metric tree partitioning. */
      DistSplitTask<MPINODE> mpisplittask;
      tree::SplitTask<NODE>  seqsplittask;
      for ( size_t t = 0; t < n_tree; t ++ )
      {
        DistTraverseDown( mpisplittask );
        LocaTraverseDown( seqsplittask );
        ExecuteAllTasks();
        
        /** Query neighbors computed in CIDS distribution.  */
        DistData<STAR, CIDS, pair<T, size_t>> Q_cids( k, this->n, this->treelist[ 0 ]->gids, initNN, this->GetComm() );
        /** Pass in neighbor pointer. */
        this->setup.NN = &Q_cids;
        LocaTraverseLeafs( dummy );
        ExecuteAllTasks();

        /** Queries computed in CBLK distribution */
        DistData<STAR, CBLK, pair<T, size_t>> Q_cblk( k, this->n, this->GetComm() );
        /** Redistribute from CIDS to CBLK */
        Q_cblk = Q_cids;
        /** Merge Q_cblk into NN (sort and remove duplication) */
        assert( Q_cblk.col_owned() == NN.col_owned() );
        MergeNeighbors( k, NN.col_owned(), NN, Q_cblk );
      }







//      /** Metric tree partitioning. */
//      DistSplitTask<MPINODE> mpisplittask;
//      tree::SplitTask<NODE>  seqsplittask;
//      DependencyCleanUp();
//      DistTraverseDown( mpisplittask );
//      LocaTraverseDown( seqsplittask );
//      ExecuteAllTasks();
//
//
//      for ( size_t t = 0; t < n_tree; t ++ )
//      {
//        this->Barrier();
//        //if ( this->GetCommRank() == 0 ) printf( "Iteration #%lu\n", t );
//
//        /** Query neighbors computed in CIDS distribution.  */
//        DistData<STAR, CIDS, pair<T, size_t>> Q_cids( k, this->n, 
//            this->treelist[ 0 ]->gids, initNN, this->GetComm() );
//        /** Pass in neighbor pointer. */
//        this->setup.NN = &Q_cids;
//        /** Overlap */
//        if ( t != n_tree - 1 )
//        {
//          //DependencyCleanUp();
//          DistTraverseDown( mpisplittask );
//          ExecuteAllTasks();
//        }
//        mpi::PrintProgress( "[MID] Here ...", this->GetComm() );
//        DependencyCleanUp();
//        LocaTraverseLeafs( dummy );
//        LocaTraverseDown( seqsplittask );
//        ExecuteAllTasks();
//        mpi::PrintProgress( "[MID] Here 22...", this->GetComm() );
//
//        if ( t == 0 )
//        {
//          /** Redistribute from CIDS to CBLK */
//          NN = Q_cids; 
//        }
//        else
//        {
//          /** Queries computed in CBLK distribution */
//          DistData<STAR, CBLK, pair<T, size_t>> Q_cblk( k, this->n, this->GetComm() );
//          /** Redistribute from CIDS to CBLK */
//          Q_cblk = Q_cids;
//          /** Merge Q_cblk into NN (sort and remove duplication) */
//          assert( Q_cblk.col_owned() == NN.col_owned() );
//          MergeNeighbors( k, NN.col_owned(), NN, Q_cblk );
//        }
//
//        //double mer_time = omp_get_wtime() - beg;
//
//        //if ( rank == 0 )
//        //printf( "%lfs %lfs %lfs\n", mpi_time, seq_time, mer_time ); fflush( stdout );
//      }

      /** Check for illegle values. */
      for ( auto &neig : NN )
      {
        if ( neig.second < 0 || neig.second >= NN.col() )
        {
          printf( "Illegle neighbor gid %lu\n", neig.second );
          break;
        }
      }

      mpi::PrintProgress( "[END] NeighborSearch ...", this->GetComm() );
      return NN;
    }; /** end AllNearestNeighbor() */

    


    /** @brief partition n points using a distributed binary tree. */ 
    void TreePartition() 
    {
      mpi::PrintProgress( "[BEG] TreePartitioning ...", this->GetComm() );

      /** Set up total problem size n and leaf node size m. */
      this->n = this->setup.ProblemSize();
      this->m = this->setup.LeafNodeSize();

      /** Initial gids distribution (asssuming Round-Robin). */
      //for ( size_t i = rank; i < this->n; i += size ) 
      for ( size_t i = this->GetCommRank(); i < this->n; i += this->GetCommSize() ) 
        this->global_indices.push_back( i );
      /** Local problem size (assuming Round-Robin). */
      num_points_owned = this->global_indices.size();
      /** Allocate distributed tree nodes in advance. */
      AllocateNodes( this->global_indices );



      DependencyCleanUp();



      
      DistSplitTask<MPINODE> mpiSPLITtask;
      tree::SplitTask<NODE> seqSPLITtask;
      DistTraverseDown( mpiSPLITtask );
      LocaTraverseDown( seqSPLITtask );
      ExecuteAllTasks();



      tree::IndexPermuteTask<NODE> seqINDXtask;
      LocaTraverseUp( seqINDXtask );
      DistIndexPermuteTask<MPINODE> mpiINDXtask;
      DistTraverseUp( mpiINDXtask );
      ExecuteAllTasks();

      //printf( "rank %d finish split\n", rank ); fflush( stdout );


      /** Allocate space for point MortonID. */
      (this->setup).morton.resize( this->n );

      /** Compute Morton ID for both distributed and local trees. */
      RecursiveMorton( mpitreelists[ 0 ], MortonHelper::Root() );

      /** Clean up the map. */
      this->morton2node.clear();

      /** Construct morton2node map for the local tree. */
      for ( auto node : this->treelist ) this->morton2node[ node->morton ] = node;

      /**Construc morton2node map for the distributed tree. */ 
      for ( auto node : this->mpitreelists ) 
      {
        this->morton2node[ node->morton ] = node;
        auto *sibling = node->sibling;
        if ( node->l ) this->morton2node[ sibling->morton ] = sibling;
      }

      this->Barrier();
      mpi::PrintProgress( "[END] TreePartitioning ...", this->GetComm() ); 
    }; /** end TreePartition() */


    /** Assign MortonID to each node recursively. */
    void RecursiveMorton( MPINODE *node, MortonHelper::Recursor r ) 
    {
      /** MPI Support. */ 
      int comm_size = this->GetCommSize();
      int comm_rank = this->GetCommRank();
      int node_size = node->GetCommSize(); 
      int node_rank = node->GetCommRank(); 

      /** Set the node MortonID. */
      node->morton = MortonHelper::MortonID( r );
      /** Set my sibling's MortonID. */
      if ( node->sibling )
        node->sibling->morton = MortonHelper::SiblingMortonID( r );

      if ( node_size < 2 )
      {
        /** Compute MortonID recursively for the local tree. */
        tree::Tree<SETUP, NODEDATA>::RecursiveMorton( node, r );
        /** Prepare to exchange all <gid,MortonID> pairs. */
        auto &gids = this->treelist[ 0 ]->gids;
        vector<int> recv_size( comm_size, 0 );
        vector<int> recv_disp( comm_size, 0 );
        vector<pair<size_t, size_t>> send_pairs;
        vector<pair<size_t, size_t>> recv_pairs( this->n );

        /** Gather pairs I own. */ 
        for ( auto it : gids ) 
        {
          send_pairs.push_back( 
              pair<size_t, size_t>( it, this->setup.morton[ it ]) );
        }

        /** Exchange send_pairs.size(). */
        int send_size = send_pairs.size();
        mpi::Allgather( &send_size, 1, recv_size.data(), 1, this->GetComm() );
        /** Compute displacement for Allgatherv. */
        for ( size_t p = 1; p < comm_size; p ++ )
        {
          recv_disp[ p ] = recv_disp[ p - 1 ] + recv_size[ p - 1 ];
        }
        /** Sanity check for the total size. */
        size_t total_gids = 0;
        for ( size_t p = 0; p < comm_size; p ++ ) 
        {
          total_gids += recv_size[ p ];
        }
        assert( total_gids == this->n );
        /** Exchange all pairs. */
        mpi::Allgatherv( send_pairs.data(), send_size, 
            recv_pairs.data(), recv_size.data(), recv_disp.data(), this->GetComm() );
        /** Fill in all MortonIDs. */
        for ( auto it : recv_pairs ) this->setup.morton[ it.first ] = it.second;
      }
      else
      {
        if ( node_rank < node_size / 2 )
        {
          RecursiveMorton( node->child, MortonHelper::RecurLeft( r ) );
        }
        else                   
        {
          RecursiveMorton( node->child, MortonHelper::RecurRight( r ) );
        }
      }
    }; /** end RecursiveMorton() */




    Data<int> CheckAllInteractions()
    {
      /** Get the total depth of the tree. */
      int total_depth = this->treelist.back()->l;
      /** Number of total leaf nodes. */
      int num_leafs = 1 << total_depth;
      /** Create a 2^l-by-2^l table to check all interactions. */
      Data<int> A( num_leafs, num_leafs, 0 );
      Data<int> B( num_leafs, num_leafs, 0 );
      /** Now traverse all tree nodes (excluding the root). */
      for ( int t = 1; t < this->treelist.size(); t ++ )
      {
        auto *node = this->treelist[ t ];
        ///** Loop over all near interactions. */
        //for ( auto it : node->NNNearNodeMortonIDs )
        //{
        //  auto I = MortonHelper::Morton2Offsets( node->morton, total_depth );
        //  auto J = MortonHelper::Morton2Offsets(   it, total_depth );
        //  for ( auto i : I ) for ( auto j : J ) A( i, j ) += 1; 
        //}
        ///** Loop over all far interactions. */
        //for ( auto it : node->NNFarNodeMortonIDs )
        //{
        //  auto I = MortonHelper::Morton2Offsets( node->morton, total_depth );
        //  auto J = MortonHelper::Morton2Offsets(   it, total_depth );
        //  for ( auto i : I ) for ( auto j : J ) A( i, j ) += 1; 
        //}

        for ( int p = 0; p < this->GetCommSize(); p ++ )
        {
          if ( node->isleaf )
          {
            for ( auto & it : node->DistNear[ p ] )
            {
              auto I = MortonHelper::Morton2Offsets( node->morton, total_depth );
              auto J = MortonHelper::Morton2Offsets(   it.first, total_depth );
              for ( auto i : I ) for ( auto j : J ) 
              {
                assert( i < num_leafs && j < num_leafs );
                A( i, j ) += 1; 
              }
            }
          }
          for ( auto & it : node->DistFar[ p ] )
          {
            auto I = MortonHelper::Morton2Offsets( node->morton, total_depth );
            auto J = MortonHelper::Morton2Offsets(   it.first, total_depth );
            for ( auto i : I ) for ( auto j : J ) 
            {
              assert( i < num_leafs && j < num_leafs );
              A( i, j ) += 1; 
            }
          }
        }
      }

      for ( auto *node : mpitreelists )
      {
        ///** Loop over all near interactions. */
        //for ( auto it : node->NNNearNodeMortonIDs )
        //{
        //  auto I = MortonHelper::Morton2Offsets( node->morton, total_depth );
        //  auto J = MortonHelper::Morton2Offsets(   it, total_depth );
        //  for ( auto i : I ) for ( auto j : J ) A( i, j ) += 1; 
        //}
        ///** Loop over all far interactions. */
        //for ( auto it : node->NNFarNodeMortonIDs )
        //{
        //  auto I = MortonHelper::Morton2Offsets( node->morton, total_depth );
        //  auto J = MortonHelper::Morton2Offsets(   it, total_depth );
        //  for ( auto i : I ) for ( auto j : J ) A( i, j ) += 1; 
        //}
        for ( int p = 0; p < this->GetCommSize(); p ++ )
        {
          if ( node->isleaf )
          {
          for ( auto & it : node->DistNear[ p ] )
          {
            auto I = MortonHelper::Morton2Offsets( node->morton, total_depth );
            auto J = MortonHelper::Morton2Offsets(   it.first, total_depth );
            for ( auto i : I ) for ( auto j : J ) 
            {
              assert( i < num_leafs && j < num_leafs );
              A( i, j ) += 1; 
            }
          }
          }
          for ( auto & it : node->DistFar[ p ] )
          {
            auto I = MortonHelper::Morton2Offsets( node->morton, total_depth );
            auto J = MortonHelper::Morton2Offsets(   it.first, total_depth );
            for ( auto i : I ) for ( auto j : J ) 
            {
              assert( i < num_leafs && j < num_leafs );
              A( i, j ) += 1; 
            }
          }
        }
      }

      /** Reduce */
      mpi::Reduce( A.data(), B.data(), A.size(), MPI_SUM, 0, this->GetComm() );

      if ( this->GetCommRank() == 0 )
      {
        for ( size_t i = 0; i < num_leafs; i ++ )
        {
          for ( size_t j = 0; j < num_leafs; j ++ ) printf( "%d", B( i, j ) );
          printf( "\n" );
        }
      }

      return B;
    }; /** end CheckAllInteractions() */













    /** */ 
    int Morton2Rank( size_t it )
    {
      return MortonHelper::Morton2Rank( it, this->GetCommSize() );
    }; /** end Morton2Rank() */

    int Index2Rank( size_t gid )
    {
       return Morton2Rank( this->setup.morton[ gid ] );
    }; /** end Morton2Rank() */





    template<typename TASK, typename... Args>
    void LocaTraverseUp( TASK &dummy, Args&... args )
    {
      /** contain at lesat one tree node */
      assert( this->treelist.size() );

      /** 
       *  traverse the local tree without the root
       *
       *  IMPORTANT: local root alias of the distributed leaf node
       *  IMPORTANT: here l must be int, size_t will wrap over 
       *
       */

			//printf( "depth %lu\n", this->depth ); fflush( stdout );

      for ( int l = this->depth; l >= 1; l -- )
      {
        size_t n_nodes = 1 << l;
        auto level_beg = this->treelist.begin() + n_nodes - 1;

        /** loop over each node at level-l */
        for ( size_t node_ind = 0; node_ind < n_nodes; node_ind ++ )
        {
          auto *node = *(level_beg + node_ind);
          RecuTaskSubmit( node, dummy, args... );
        }
      }
    }; /** end LocaTraverseUp() */


    template<typename TASK, typename... Args>
    void DistTraverseUp( TASK &dummy, Args&... args )
    {
      MPINODE *node = mpitreelists.back();
      while ( node )
      {
        if ( this->DoOutOfOrder() ) RecuTaskSubmit(  node, dummy, args... );
        else                        RecuTaskExecute( node, dummy, args... );
        /** move to its parent */
        node = (MPINODE*)node->parent;
      }
    }; /** end DistTraverseUp() */


    template<typename TASK, typename... Args>
    void LocaTraverseDown( TASK &dummy, Args&... args )
    {
      /** contain at lesat one tree node */
      assert( this->treelist.size() );

      /** 
       *  traverse the local tree without the root
       *
       *  IMPORTANT: local root alias of the distributed leaf node
       *  IMPORTANT: here l must be int, size_t will wrap over 
       *
       */
      for ( int l = 1; l <= this->depth; l ++ )
      {
        size_t n_nodes = 1 << l;
        auto level_beg = this->treelist.begin() + n_nodes - 1;

        for ( size_t node_ind = 0; node_ind < n_nodes; node_ind ++ )
        {
          auto *node = *(level_beg + node_ind);
          RecuTaskSubmit( node, dummy, args... );
        }
      }
    }; /** end LocaTraverseDown() */


    template<typename TASK, typename... Args>
    void DistTraverseDown( TASK &dummy, Args&... args )
    {
      auto *node = mpitreelists.front();
      while ( node )
      {
				//printf( "now at level %lu\n", node->l ); fflush( stdout );
        if ( this->DoOutOfOrder() ) RecuTaskSubmit(  node, dummy, args... );
        else                        RecuTaskExecute( node, dummy, args... );
				//printf( "RecuTaskSubmit at level %lu\n", node->l ); fflush( stdout );

        /** 
         *  move to its child 
         *  IMPORTANT: here we need to cast the pointer back to mpitree::Node*
         */
        node = node->child;
      }
    }; /** end DistTraverseDown() */


    template<typename TASK, typename... Args>
    void LocaTraverseLeafs( TASK &dummy, Args&... args )
    {
      /** contain at lesat one tree node */
      assert( this->treelist.size() );

      int n_nodes = 1 << this->depth;
      auto level_beg = this->treelist.begin() + n_nodes - 1;

      for ( int node_ind = 0; node_ind < n_nodes; node_ind ++ )
      {
        auto *node = *(level_beg + node_ind);
        RecuTaskSubmit( node, dummy, args... );
      }
    }; /** end LocaTraverseLeaf() */


    /**
     *  @brief For unordered traversal, we just call local
     *         downward traversal.
     */ 
    template<typename TASK, typename... Args>
    void LocaTraverseUnOrdered( TASK &dummy, Args&... args )
    {
      LocaTraverseDown( dummy, args... );
    }; /** end LocaTraverseUnOrdered() */


    /**
     *  @brief For unordered traversal, we just call distributed
     *         downward traversal.
     */ 
    template<typename TASK, typename... Args>
    void DistTraverseUnOrdered( TASK &dummy, Args&... args )
    {
      DistTraverseDown( dummy, args... );
    }; /** end DistTraverseUnOrdered() */





    void DependencyCleanUp()
    {
      for ( auto node : mpitreelists ) node->DependencyCleanUp();
      //for ( size_t i = 0; i < mpitreelists.size(); i ++ )
      //{
      //  mpitreelists[ i ]->DependencyCleanUp();
      //}

      tree::Tree<SETUP, NODEDATA>::DependencyCleanUp();






      for ( auto p : NearRecvFrom ) p.DependencyCleanUp();
      for ( auto p :  FarRecvFrom ) p.DependencyCleanUp();

      /** TODO also clean up the LET node */

    }; /** end DependencyCleanUp() */


    /** @brief */
    void ExecuteAllTasks()
    {
      hmlp_run();
      this->Barrier();
      DependencyCleanUp();
    }; /** end ExecuteAllTasks() */

    /** @brief */
    void DependOnNearInteractions( int p, Task *task )
    {
      /** Describe the dependencies of rank p. */
      for ( auto it : NearSentToRank[ p ] )
      {
        auto *node = this->morton2node[ it ];
        node->DependencyAnalysis( R, task );
      }
      /** Try to enqueue if there is no dependency. */
      task->TryEnqueue();
    }; /** end DependOnNearInteractions() */

    /** @brief */
    void DependOnFarInteractions( int p, Task *task )
    {
      /** Describe the dependencies of rank p. */
      for ( auto it : FarSentToRank[ p ] )
      {
        auto *node = this->morton2node[ it ];
        node->DependencyAnalysis( R, task );
      }
      /** Try to enqueue if there is no dependency. */
      task->TryEnqueue();
    }; /** end DependOnFarInteractions() */



    /**
     *  Interaction lists per rank
     *
     *  NearSentToRank[ p ]   contains all near node MortonIDs sent   to rank p.
     *  NearRecvFromRank[ p ] contains all near node MortonIDs recv from rank p.
     *  NearRecvFromRank[ p ][ morton ] = offset in the received vector.
     */
    vector<vector<size_t>>   NearSentToRank;
    vector<map<size_t, int>> NearRecvFromRank;
    vector<vector<size_t>>   FarSentToRank;
    vector<map<size_t, int>> FarRecvFromRank;

    vector<ReadWrite> NearRecvFrom;
    vector<ReadWrite> FarRecvFrom;
    
  private:

    /** global communicator error message. */
    int ierr = 0;
    /** n = sum( num_points_owned ) from all MPI processes. */
    size_t num_points_owned = 0;

}; /** end class Tree */


}; /** end namespace mpitree */
}; /** end namespace hmlp */

#endif /** define MPITREE_HPP */
