#ifndef FILE_ORDER
#define FILE_ORDER

/* *************************************************************************/
/* File:   order.hh                                                       */
/* Author: Joachim Schoeberl                                              */
/* Date:   18. Jun. 97                                                    */
/* *************************************************************************/


namespace ngla
{

  /*
    reordering for sparse cholesky factoriztion
  */

  ///
  class MDOPriorityQueue
  {
    struct entry { 
      int degree, prev, next;
    };
    Array<entry> list;
    Array<int> first_in_class;
  public:
    MDOPriorityQueue (int size, int maxdeg);
    ~MDOPriorityQueue ();
    int MinDegree () const;
    int GetDegree (int nr) const { return list[nr].degree; }
    void SetDegree (int nr, int deg);
    void Invalidate (int nr);
  };


  ///
  class MDOVertex
  {
  protected:
    int master;         /// master of node
    int nextslave;      /// linked list of slaves
    int numslaves;      /// number of slaves
    int numcliques;     /// number of cliques
    bool eliminated;    /// node is eliminated
    bool used;          /// temporary field (used in calcorder)
    bool flag;          
    

  public:
    ///
    int * connected = nullptr;
    ///
    int nconnected;

    ///
    MDOVertex() = default;
    MDOVertex(int ma)  { Init (ma); }
    ///
    ~MDOVertex()   { ; }

    /// 
    void Init (int ma)
    {
      master = ma;
      nextslave = -1;
      numslaves = 0;
      numcliques = 0;
      flag = 0;
      eliminated = 0;
      used = 0;
    }

    ///
    int Master() const { return master; };
    ///
    void SetMaster(int ma) { master = ma; };
    ///
    int NextSlave () const {return nextslave; };
    ///
    void SetNextSlave( int ns ) { nextslave = ns; };
    ///
    bool Eliminated() const {return eliminated; };
    ///
    void SetEliminated(bool el) {eliminated = el; };
    ///
    bool Used() const {return used; };
    ///
    void SetUsed(bool us) {used = us; } ;
    ///
    bool Flag() const {return flag; };
    ///
    void SetFlag(bool fl) {flag = fl;};

    friend class MinimumDegreeOrdering;
  };

  /// 
  class CliqueEl
  {
    bool flag = false;
  public:
    CliqueEl * next = nullptr;
    CliqueEl * nextcl = nullptr;
    CliqueEl * clmaster;
    int vnr;
    bool eliminate = false;
  
    CliqueEl (int avnr) : vnr(avnr) { ; }

    CliqueEl * GetNext() { return next; }
    CliqueEl * GetNextClique() { return nextcl; }
    int GetVertexNr() const { return vnr; }
    
    void SetFlag (bool aflag) { clmaster->flag = aflag; }
    bool Flag () const { return clmaster->flag; }
    
    operator int() const { return vnr; }
    int Nr() const { return vnr; }
  };
  

  ///
  class MinimumDegreeOrdering
  {
  public:
    ///
    int n, nused;
    ///
    Array<CliqueEl*> cliques;
    ///
    Array<int> order;
    ///
    Array<int> blocknr;
    ///
    Array<MDOVertex> vertices;
    ///
    MDOPriorityQueue priqueue;
    ///
    ngstd::BlockAllocator ball;
  public:
    ///
    MinimumDegreeOrdering (int an);
    ///
    void AddEdge (int v1, int v2);
    ///
    void PrintCliques ();

    ///
    int CalcDegree (int v1);
    ///
    void EliminateMasterVertex (int v);
    void EliminateSlaveVertex (int v);
    ///
    void Order();
    /// 
    ~MinimumDegreeOrdering();

    ///
    int NumCliques (int v) const
    {
      return vertices[v].numcliques;
    }

    ///
    void SetUnusedVertex (int v) { vertices[v].SetEliminated(true); order[v] = -1; }
      
    /// set/clear flag for all nodes in clique
    void SetFlagNodes (int v);
    ///
    void ClearFlagNodes (int v);
  
    /// set/clear flag in all cliques of node
    void SetFlagCliques (int v);
    ///
    void ClearFlagCliques (int v);
    ///
    int Size () const { return n; }
    /// number of non-zero elements
    //  int GetNZE() const;
    //  friend class SparseCholesky;

    int NextSlave (int vnr) const
    {
      return vertices[vnr].NextSlave();
    }
    ///
    int NumSlaves (int vnr) const 
    {
      return vertices[vnr].numslaves;
    }

    ///
    bool IsMaster (int vnr) const
    {
      return vertices[vnr].Master() == vnr;
    }

    void SetMaster (int master, int slave);
  };


}








#endif
