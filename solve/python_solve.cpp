#ifdef NGS_PYTHON
#include "../ngstd/python_ngstd.hpp"
#include<pybind11/numpy.h>
#include<pybind11/stl_bind.h>
#include<pybind11/stl.h>
#include <solve.hpp>
using namespace ngsolve;


typedef GridFunction GF;

extern void ExportBVP(py::module &m);
extern void ExportDrawFlux(py::module &m);
void ExportVisFunctions(py::module &m);

void NGS_DLL_HEADER ExportNgsolve(py::module &m ) {

    m.def ("Tcl_Eval", &Ng_TclCmd);

    m.def ("_Redraw",
           ([](bool blocking, double fr)
             {
               static auto last_time = std::chrono::system_clock::now()-std::chrono::seconds(10);
               auto now = std::chrono::system_clock::now();
               double elapsed = std::chrono::duration<double>(now-last_time).count();
               if (elapsed * fr > 1)
                 {
                   Ng_Redraw(blocking);
                   last_time = std::chrono::system_clock::now();
                 }
             }),
           py::arg("blocking")=false, py::arg("fr") = 25
             );


    m.def ("Draw",[](shared_ptr<MeshAccess> mesh) 
                                     {
                                       mesh->SelectMesh();
                                       Ng_TclCmd ("set ::selectvisual mesh;\n");
                                     }
             );


    m.def("SetVisualization",
            [](py::object deformation, py::object min, py::object max,
                /* py::object clippnt, */ py::object clipnormal, py::object clipping)
             {
               bool need_redraw = false;
               if (py::extract<bool>(deformation).check())
                 {
                   bool def = py::extract<bool>(deformation)();
                   Ng_TclCmd ("set ::visoptions.deformation "+ToString(def)+";\n");
                   Ng_TclCmd ("Ng_Vis_Set parameters;\n");
                   need_redraw = true;
                 }
               if (py::extract<double>(min).check())
                 {
                   Ng_TclCmd ("set ::visoptions.autoscale 0\n");
                   Ng_TclCmd ("set ::visoptions.mminval "+ToString(py::extract<double>(min)())+";\n");
                   Ng_TclCmd ("Ng_Vis_Set parameters;\n");                                      
                   need_redraw = true;
                 }
               if (py::extract<double>(max).check())
                 {
                   Ng_TclCmd ("set ::visoptions.autoscale 0\n");
                   Ng_TclCmd ("set ::visoptions.mmaxval "+ToString(py::extract<double>(max)())+";\n");
                   Ng_TclCmd ("Ng_Vis_Set parameters;\n");                   
                   need_redraw = true;
                 }
               if (py::extract<py::tuple>(clipnormal).check())
                 {
                   py::tuple norm = py::extract<py::tuple>(clipnormal)();
                   if (py::len(norm)==3)
                     {
                       // cout << "setting clipping normal" << endl;
                       // tclstring << "set ::viewoptions.clipping.enable 1" << endl
                       Ng_TclCmd ("set ::viewoptions.clipping.nx "+ToString(py::extract<double>(norm[0])())+";\n");
                       Ng_TclCmd ("set ::viewoptions.clipping.ny "+ToString(py::extract<double>(norm[1])())+";\n");
                       Ng_TclCmd ("set ::viewoptions.clipping.nz "+ToString(py::extract<double>(norm[2])())+";\n");
                       // << "set ::viewoptions.clipping.dist " << clipdist << endl;
                       need_redraw = true;
                     }
                 }
               if (py::extract<bool>(clipping).check())
                 {
                   bool clip = py::extract<bool>(clipping)();
                   Ng_TclCmd ("set ::viewoptions.clipping.enable "+ToString(int(clip))+";\n");
                   Ng_TclCmd ("Ng_SetVisParameters");
                   
                   need_redraw = true;
                 }
               if (need_redraw)
                 Ng_Redraw(true);
             },
             py::arg("deformation")=DummyArgument(),
             py::arg("min")=DummyArgument(),
             py::arg("max")=DummyArgument(),
             // py::arg("clippnt")=DummyArgument(),
             py::arg("clipnormal")=DummyArgument(),
             py::arg("clipping")=DummyArgument()
            )
      ;
    
    m.def ("Draw",
           [](shared_ptr<GridFunction> gf, int sd, bool autoscale, double min, double max)
              {
                // cout << "in draw" << endl;
                // cout << "gf in draw = " << *gf << endl;
                // cout << "dims of gf =  " << gf->Dimensions() << endl;
                gf->GetMeshAccess()->SelectMesh();
                // Visualize (gf, gf->GetName());

                // netgen::SolutionData * vis = new VisualizeCoefficientFunction (gf->GetMeshAccess(), gf);
                auto gfcf = make_shared<GridFunctionCoefficientFunction> (gf);
                netgen::SolutionData * vis = new VisualizeCoefficientFunction (gf->GetMeshAccess(), gfcf);                

                Ng_SolutionData soldata;
                Ng_InitSolutionData (&soldata);
  
                soldata.name = gf->GetName().c_str();
                soldata.data = 0;
                soldata.components = gf -> Dimension();
                if (gf->IsComplex()) soldata.components *= 2;
                soldata.iscomplex = gf -> IsComplex();
                soldata.draw_surface = true; // draw_surf;
                soldata.draw_volume  = true; // draw_vol; 
                /* 
                if (flags.GetDefineFlag("volume"))
                  soldata.draw_surface = false;
                if (flags.GetDefineFlag("boundary"))
                  soldata.draw_volume = false;
                */
                soldata.dist = 1;
                soldata.soltype = NG_SOLUTION_VIRTUAL_FUNCTION;
                soldata.solclass = vis;
                Ng_SetSolutionData (&soldata);
                
                if (gf->Dimension() == 1)
                  Ng_TclCmd (string("set ::visoptions.scalfunction ")+gf->GetName()+":1;\n");
                else
                  Ng_TclCmd (string("set ::visoptions.vecfunction ")+gf->GetName()+";\n");
                Ng_TclCmd (string("set ::visoptions.subdivisions ")+ToString(sd)+";\n");
		Ng_TclCmd ("set ::visoptions.autoscale "+ToString(autoscale)+";\n");
		if(!autoscale){
		  Ng_TclCmd ("set ::visoptions.mminval "+ToString(min)+";\n");
		  Ng_TclCmd ("set ::visoptions.mmaxval "+ToString(max)+";\n");
		}
		Ng_TclCmd ("Ng_Vis_Set parameters;\n");
                Ng_TclCmd ("set ::selectvisual solution;\n");
              },
             py::arg("gf"),py::arg("sd")=2,py::arg("autoscale")=true,
	      py::arg("min")=0.0,py::arg("max")=1.0
             );

    
    m.def ("Draw", [](shared_ptr<CoefficientFunction> cf, shared_ptr<MeshAccess> ma, string name,
                 int sd, bool autoscale, double min, double max,
                 bool draw_vol, bool draw_surf) 
              {
                ma->SelectMesh();
                netgen::SolutionData * vis;
                if(dynamic_cast<ProlongateCoefficientFunction *>(cf.get()))
                {
                  shared_ptr<CoefficientFunction> wrapper(new ProlongateCoefficientFunctionVisualization(*static_cast<ProlongateCoefficientFunction *>(cf.get())));
                  vis = new VisualizeCoefficientFunction (ma, wrapper);
                }
                else
                  vis = new VisualizeCoefficientFunction (ma, cf);
                Ng_SolutionData soldata;
                Ng_InitSolutionData (&soldata);
  
                soldata.name = (char*)name.c_str();
                soldata.data = 0;
                soldata.components = cf -> Dimension();
                if (cf->IsComplex()) soldata.components *= 2;
                soldata.iscomplex = cf -> IsComplex();
                soldata.draw_surface = draw_surf;
                soldata.draw_volume  = draw_vol; 
                /* 
                if (flags.GetDefineFlag("volume"))
                  soldata.draw_surface = false;
                if (flags.GetDefineFlag("boundary"))
                  soldata.draw_volume = false;
                */
                soldata.dist = 1;
                soldata.soltype = NG_SOLUTION_VIRTUAL_FUNCTION;
                soldata.solclass = vis;
                Ng_SetSolutionData (&soldata);
                

                if (cf->Dimension() == 1)
                  Ng_TclCmd (string("set ::visoptions.scalfunction ")+name+":1;\n");
                else
                  if (cf->Dimension() == 3 || cf->Dimension() == ma->GetDimension())
                    Ng_TclCmd (string("set ::visoptions.vecfunction ")+name+";\n");

                Ng_TclCmd (string("set ::visoptions.subdivisions ")+ToString(sd)+";\n");
		Ng_TclCmd ("set ::visoptions.autoscale "+ToString(autoscale)+";\n");
		if(!autoscale){
		  Ng_TclCmd ("set ::visoptions.mminval "+ToString(min)+";\n");
		  Ng_TclCmd ("set ::visoptions.mmaxval "+ToString(max)+";\n");
		}
                Ng_TclCmd ("Ng_Vis_Set parameters;\n");
                Ng_TclCmd ("set ::selectvisual solution;\n");

              },
              py::arg("cf"),py::arg("mesh"),py::arg("name"),
              py::arg("sd")=2,py::arg("autoscale")=true,
	      py::arg("min")=0.0,py::arg("max")=1.0,
              py::arg("draw_vol")=true,py::arg("draw_surf")=true
             );





    ExportBVP(m);
    ExportDrawFlux(m);
    ExportVisFunctions(m);
}

//////////////////////////////////////////////////////////////////////////////
// Utility functions for visualization 
//////////////////////////////////////////////////////////////////////////////

template <typename TSCAL>
static void ExtractRealImag(const TSCAL val, int i, float &real, float &imag);

template<> void ExtractRealImag(const SIMD<Complex> val, int i, float &real, float &imag) {
    real = val.real()[i];
    imag = val.imag()[i];
}

template<> void ExtractRealImag(const Complex val, int i, float &real, float &imag) {
    real = val.real();
    imag = val.imag();
}

template<> void ExtractRealImag(const SIMD<double> val, int i, float &real, float &imag) { real = val[i]; }
template<> void ExtractRealImag(const double val, int i, float &real, float &imag) { real = val; }

template<typename TSCAL, typename TMIR>
static void GetValues( const CoefficientFunction &cf, LocalHeap &lh, const TMIR &mir, FlatArray<float> values_real, FlatArray<float> values_imag , FlatArray<float> min, FlatArray<float> max) {
    static_assert( is_same<TSCAL, SIMD<double>>::value || is_same<TSCAL, SIMD<Complex>>::value || is_same<TSCAL, double>::value || is_same<TSCAL, Complex>::value, "Unsupported type in GetValues");

    HeapReset hr(lh);

    auto ncomps = cf.Dimension();
    int nip = mir.IR().GetNIP();
    auto getIndex = [=] ( int p, int comp ) { return p*ncomps + comp; };
    bool is_complex = is_same<TSCAL, SIMD<Complex>>::value || is_same<TSCAL, Complex>::value;

    if(is_same<TSCAL, SIMD<double>>::value || is_same<TSCAL, SIMD<Complex>>::value)
    {
        FlatMatrix<TSCAL> values(ncomps, mir.Size(), lh);
        cf.Evaluate(mir, values);

        constexpr int n = SIMD<double>::Size();
        for (auto k : Range(nip)) {
            for (auto i : Range(ncomps)) {
                float vreal = 0.0;
                float vimag = 0.0;
                ExtractRealImag( values(i, k/n), k%n, vreal, vimag );
                auto index = getIndex(k,i);
                values_real[index] = vreal;
                if(is_complex) {
                  values_imag[index] = vimag;
                  min[i] = min2(min[i], sqrt(vreal*vreal+vimag+vimag));
                  max[i] = max2(max[i], sqrt(vreal*vreal+vimag+vimag));
                }
                else {
                  min[i] = min2(min[i], vreal);
                  max[i] = max2(max[i], vreal);
                }
            }
        }
    }
    else
    {
        FlatMatrix<TSCAL> values(mir.Size(), ncomps, lh);
        cf.Evaluate(mir, values);
        for (auto k : Range(nip)) {
            for (auto i : Range(ncomps)) {
                float vreal = 0.0;
                float vimag = 0.0;
                ExtractRealImag( values(k,i), 0, vreal, vimag );
                auto index = getIndex(k,i);
                values_real[index] = vreal;
                if(is_complex) {
                  values_imag[index] = vimag;
                  min[i] = min2(min[i], sqrt(vreal*vreal+vimag+vimag));
                  max[i] = max2(max[i], sqrt(vreal*vreal+vimag+vimag));
                }
                else {
                  min[i] = min2(min[i], vreal);
                  max[i] = max2(max[i], vreal);
                }
            }
        }
    }

}

template<int S, int R>
static BaseMappedIntegrationRule &T_GetMappedIR (IntegrationRule & ir, ElementTransformation & eltrans, LocalHeap &lh ) {
    void *p = lh.Alloc(sizeof(MappedIntegrationRule<S,R>));
    return *new (p) MappedIntegrationRule<S,R> (ir, eltrans, lh);
}

template<int S, int R>
static SIMD_BaseMappedIntegrationRule &T_GetMappedIR (SIMD_IntegrationRule & ir, ElementTransformation & eltrans, LocalHeap &lh ) {
    void *p = lh.Alloc(sizeof(SIMD_MappedIntegrationRule<S,R>));
    return *new (p) SIMD_MappedIntegrationRule<S,R> (ir, eltrans, lh);
}


template<typename TIR>
static auto &GetMappedIR (shared_ptr<MeshAccess> ma, Ngs_Element &el, TIR & ir, LocalHeap &lh ) {
    ElementTransformation & eltrans = ma->GetTrafo (el, lh);
    const int dim = ma->GetDimension();
    const auto vb = el.VB();

    if(dim==1) {
        if(vb==VOL) return T_GetMappedIR<1,1>(ir, eltrans, lh);
    }
    if(dim==2) {
        if(vb==BND) return T_GetMappedIR<1,2>(ir, eltrans, lh);
        if(vb==VOL) return T_GetMappedIR<2,2>(ir, eltrans, lh);
    }
    if(dim==3) {
        if(vb==BBND) return T_GetMappedIR<1,3>(ir, eltrans, lh);
        if(vb==BND) return T_GetMappedIR<2,3>(ir, eltrans, lh);
        if(vb==VOL) return T_GetMappedIR<3,3>(ir, eltrans, lh);
    }
    throw Exception("GetMappedIR: unknown dimension/VorB combination: " + ToString(vb) + ","+ToString(dim));
}

void ExportVisFunctions(py::module &m) {
    m.def("_GetVisualizationData", [] (shared_ptr<ngcomp::MeshAccess> ma, map<ngfem::ELEMENT_TYPE, IntegrationRule> irs) {
            struct ElementInformation {
                ElementInformation( ngfem::ELEMENT_TYPE type_, bool curved_=false)
                  : type(type_), curved(curved_), nelements(0) { }
                Array<int> data; // the data that will go into the gpu texture buffer
                ngfem::ELEMENT_TYPE type;
                bool curved;
                int nelements;
            };

            auto getVB = [](int codim) {
                switch(codim) {
                  case 0: return VOL;
                  case 1: return BND;
                  case 2: return BBND;
                  case 3: return BBBND;
                  default: throw Exception("invalid codim");
                }
            };

            Vector<> min(3);
            min = std::numeric_limits<double>::max();
            Vector<> max(3);
            max = std::numeric_limits<double>::lowest();

            ngstd::Array<float> vertices;
            vertices.SetAllocSize(ma->GetNV()*3);
            for ( auto vi : Range(ma->GetNV()) ) {
                auto v = ma->GetPoint<3>(vi);
                for (auto i : Range(3)) {
                  vertices.Append(v[i]);
                  min[i] = min2(min[i], v[i]);
                  max[i] = max2(max[i], v[i]);
                }
            }

            LocalHeap lh(1000000, "GetMeshData");
            auto toDict = [] (ElementInformation &ei) {
              py::dict res;
              res["data"] = py::cast(ei.data);
              res["type"] = py::cast(ei.type);
              res["curved"] = py::cast(ei.curved);
              res["nelements"] = py::cast(ei.nelements);
              return res;
            };

            const int dim = ma->GetDimension();

            std::map<VorB, py::list> element_data;
            ElementInformation points(ET_POINT);
            points.nelements = ma->GetNV();
            element_data[getVB(dim)].append(toDict(points));

            ElementInformation edges(ET_SEGM);

            if(dim>=2) {
                // collect edges
                edges.nelements = ma->GetNEdges();
                edges.data.SetAllocSize(edges.nelements*4);
                // Edges of mesh (skip this for dim==1, in this case edges are treated as volume elements below)
                for (auto nr : Range(ma->GetNEdges())) {
                    auto pair = ma->GetEdgePNums(nr);
                    edges.data.Append({int(nr), int(-1), int(pair[0]), int(pair[1])});
                }
            }

            ElementInformation periodic_vertices(ET_SEGM);
            int n_periodic_vertices = ma->GetNPeriodicNodes(NT_VERTEX);
            periodic_vertices.nelements = n_periodic_vertices;
            periodic_vertices.data.SetAllocSize(periodic_vertices.nelements*4);
            for(auto idnr : Range(ma->GetNPeriodicIdentifications()))
                for (const auto& pair : ma->GetPeriodicNodes(NT_VERTEX, idnr))
                    periodic_vertices.data.Append({idnr, -1, pair[0],pair[1]});

            if(dim>=1) {
                ElementInformation edges[2] = { {ET_SEGM}, {ET_SEGM, true } };

                // 1d Elements
                VorB vb = getVB(dim-1);
                for (auto el : ma->Elements(vb)) {
                    auto verts = el.Vertices();
                    auto &ei = edges[el.is_curved];

                    ei.nelements++;
                    ei.data.Append({int(el.Nr()), int(el.GetIndex()), int(verts[0]), int(verts[1])});
                    if(el.is_curved) {
                        ei.data.Append(vertices.Size()/3);

                        HeapReset hr(lh);
                        IntegrationRule &ir = irs[el.GetType()];
                        auto & mir = GetMappedIR(ma, el, ir, lh);
                        // normals of corner vertices
                        for (auto j : ngcomp::Range(2)) {
                            auto p = static_cast<DimMappedIntegrationPoint<1>&>(mir[j]);
                            auto n = p.GetNV();
                            for (auto i : Range(3))
                                vertices.Append(n[i]);
                        }
                        // mapped coordinates of midpoint (for P2 interpolation)
                        auto p = mir[2].GetPoint();
                        for (auto i : Range(3))
                            vertices.Append(p[i]);
                    }
                }
                if(edges[0].nelements>0) element_data[vb].append(toDict(edges[0]));
                if(edges[1].nelements>0) element_data[vb].append(toDict(edges[1]));
            }
            if(dim>=2) {
                // 2d Elements
                ElementInformation trigs[2] = { {ET_TRIG}, {ET_TRIG, true } };
                ElementInformation quads[2] = { {ET_QUAD}, {ET_QUAD, true } };

                VorB vb = getVB(dim-2);
                for (auto el : ma->Elements(vb)) {
                    auto verts = el.Vertices();
                    auto nverts = verts.Size();
                    auto &ei = (nverts==3) ? trigs[el.is_curved] : quads[el.is_curved];
                    ei.nelements++;
                    ei.data.Append(el.Nr());
                    ei.data.Append(el.GetIndex());
                    for (auto i : Range(nverts))
                        ei.data.Append(verts[i]);

                    if(el.is_curved) {
                        ei.data.Append(vertices.Size()/3);
                        HeapReset hr(lh);
                        IntegrationRule &ir = irs[el.GetType()];
                        auto & mir = GetMappedIR(ma, el, ir, lh);
                        // normals of corner vertices
                        for (auto j : ngcomp::Range(nverts)) {
                            Vec<3> n(0,0,1);
                            if(vb==BND)
                                n = static_cast<DimMappedIntegrationPoint<3>&>(mir[j]).GetNV();
                            for (auto i : Range(3))
                                vertices.Append(n[i]);
                        }
                        // mapped coordinates of edge midpoints (for P2 interpolation)
                        for (auto j : ngcomp::Range(nverts,ir.Size())) {
                            auto p = mir[j].GetPoint();
                            for (auto i : Range(3))
                                vertices.Append(p[i]);
                        }
                    }
                }

                for (auto i : Range(2)) {
                  if(trigs[i].data.Size()) element_data[vb].append(toDict(trigs[i]));
                  if(quads[i].data.Size()) element_data[vb].append(toDict(quads[i]));
                }
            }

            if(dim==3) {
                ElementInformation tets[2] = { {ET_TET}, {ET_TET, true } };
                ElementInformation pyramids[2] = { {ET_PYRAMID}, {ET_PYRAMID, true } };
                ElementInformation prisms[2] = { {ET_PRISM}, {ET_PRISM, true } };
                ElementInformation hexes[2] = { {ET_HEX}, {ET_HEX, true } };

                for (auto el : ma->Elements(VOL)) {
                    auto verts = el.Vertices();
                    auto nverts = verts.Size();
                    ElementInformation * pei;
                    IntegrationRule &ir = irs[el.GetType()];

                    switch(nverts) {
                      case 4: pei = tets; break;
                      case 5: pei = pyramids; break;
                      case 6: pei = prisms; break;
                      case 8: pei = hexes; break;
                      default:
                        throw Exception("GetMeshData(): unknown element");
                    }
                    ElementInformation &ei = pei[el.is_curved];
                    ei.nelements++;
                    ei.data.Append(el.Nr());
                    ei.data.Append(el.GetIndex());
                    for (auto v : verts)
                        ei.data.Append(v);

                    if(el.is_curved) {
                        ei.data.Append(vertices.Size()/3);
                        HeapReset hr(lh);
                        auto & mir = GetMappedIR(ma, el, ir, lh);
                        // mapped coordinates of edge midpoints (for P2 interpolation)
                        for (auto &ip : mir) {
                          auto p = ip.GetPoint();
                          for (auto i : Range(3))
                              vertices.Append(p[i]);
                        }
                    }
                }
                for (auto i : Range(2)) {
                    if(tets[i].data.Size()) element_data[VOL].append(toDict(tets[i]));
                    if(pyramids[i].data.Size()) element_data[VOL].append(toDict(pyramids[i]));
                    if(prisms[i].data.Size()) element_data[VOL].append(toDict(prisms[i]));
                    if(hexes[i].data.Size()) element_data[VOL].append(toDict(hexes[i]));
                }
            }
            py::dict py_eldata;

            py::list py_edges;
            py_edges.append(toDict(edges));
            py_eldata["edges"] = py_edges;

            py::list py_periodic_vertices;
            py_periodic_vertices.append(toDict(periodic_vertices));
            py_eldata["periodic"] = py_periodic_vertices;

            py::object bbbnd = py::cast(BBBND);
            py_eldata[py::cast(BBND)] = element_data[BBBND];
            py_eldata[py::cast(BBND)] = element_data[BBND];
            py_eldata[py::cast(BND)] = element_data[BND];
            py_eldata[py::cast(VOL)] = element_data[VOL];

            py_eldata["min"] = min;
            py_eldata["max"] = max;
            py_eldata["vertices"] = MoveToNumpyArray(vertices);
            return py_eldata;
         });

    m.def("_GetValues", [] (shared_ptr<ngfem::CoefficientFunction> cf, shared_ptr<ngcomp::MeshAccess> ma, VorB vb, map<ngfem::ELEMENT_TYPE, IntegrationRule> irs) {
              auto tm = task_manager;
              task_manager = nullptr;
              LocalHeap lh(10000000, "GetValues");
              int dim = ma->GetDimension();
              if(vb==BND) dim-=1;

              map<ngfem::ELEMENT_TYPE, SIMD_IntegrationRule> simd_irs;
              for (auto & p : irs ) {
                simd_irs[p.first] = p.second;
              }
              typedef std::pair<ngfem::ELEMENT_TYPE,bool> T_ET;
              map<T_ET, Array<float>> values_real;
              map<T_ET, Array<float>> values_imag;

              int ncomps = cf->Dimension();
              Array<float> min(ncomps);
              Array<float> max(ncomps);
              min = std::numeric_limits<float>::max();
              max = std::numeric_limits<float>::lowest();

              map<T_ET, std::atomic<int>> element_counter;
              map<T_ET, std::atomic<int>> element_index;
              for (auto et : {ET_POINT, ET_SEGM, ET_TRIG, ET_QUAD, ET_TET, ET_PRISM, ET_PYRAMID, ET_HEX}) {
                for (auto curved : {false, true}) {
                  element_counter[T_ET{et,curved}] = 0;
                  element_index[T_ET{et,curved}] = 0;
                }
              }
              ma->IterateElements(vb, lh,[&](auto el, LocalHeap& mlh) {
                  auto et = el.GetType();
                  element_counter[T_ET{et,el.is_curved}]++;
              });

              bool use_simd = true;
              ma->IterateElements(vb, lh,[&](auto el, LocalHeap& mlh) {
                  FlatArray<float> min_local(ncomps, mlh);
                  FlatArray<float> max_local(ncomps, mlh);
                  auto curved = el.is_curved;

                  auto et = el.GetType();
                  auto & ir = irs[et];
                  auto & simd_ir = simd_irs[et];
                  auto &vals_real = values_real[T_ET{et,curved}];
                  auto &vals_imag = values_imag[T_ET{et,curved}];
                  int nip = irs[et].GetNIP();
                  int values_per_element = nip*ncomps;
                  size_t first = vals_real.Size();
                  size_t next = first + values_per_element;
                  vals_real.SetSize(next);
                  if(cf->IsComplex()) vals_imag.SetSize(next);
                  if(use_simd)
                    {
                      try
                        {
                          auto & mir = GetMappedIR(ma, el, simd_ir, lh);
                          if(cf->IsComplex())
                            GetValues<SIMD<Complex>>( *cf, mlh, mir, vals_real.Range(first,next), vals_imag.Range(first,next), min_local, max_local);
                          else
                            GetValues<SIMD<double>>( *cf, mlh, mir, vals_real.Range(first,next), vals_imag, min_local, max_local);
                        }
                      catch(ExceptionNOSIMD e)
                        {
                          use_simd = false;
                        }
                    }
                  if(!use_simd)
                    {
                      auto & mir = GetMappedIR(ma, el, ir, lh);
                      if(cf->IsComplex())
                        GetValues<Complex>( *cf, mlh, mir, vals_real.Range(first,next), vals_imag.Range(first,next), min_local, max_local);
                      else
                        GetValues<double>( *cf, mlh, mir, vals_real.Range(first,next), vals_imag, min_local, max_local);
                    }
                  for (auto i : Range(ncomps)) {
                      float expected = min[i];
                      while (min_local[i] < expected)
                          AsAtomic(min[i]).compare_exchange_weak(expected, min_local[i], std::memory_order_relaxed, std::memory_order_relaxed);
                      expected = max[i];
                      while (max_local[i] > expected)
                          AsAtomic(max[i]).compare_exchange_weak(expected, max_local[i], std::memory_order_relaxed, std::memory_order_relaxed);
                  }


                });
            py::gil_scoped_acquire ac;
            py::dict res_real;
            py::dict res_imag;
            for (auto &p : irs) {
                auto et = p.first;
                for (auto curved : {false, true}) {
                  if (values_real[T_ET{et,curved}].Size()>0) {
                    res_real[py::make_tuple(et,curved)] = MoveToNumpyArray(values_real[T_ET{et,curved}]);
                      if(cf->IsComplex())
                        res_imag[py::make_tuple(et,curved)] = MoveToNumpyArray(values_imag[T_ET{et,curved}]);
                    }
                  }
            }
            py::dict res;
            res["min"] = MoveToNumpyArray(min);
            res["max"] = MoveToNumpyArray(max);
            res["real"] = res_real;
            res["imag"] = res_imag;
            task_manager = tm;
            return res;
        },py::call_guard<py::gil_scoped_release>());

    m.def("_SetLocale", []()
          {
            setlocale(LC_NUMERIC,"C");
          });
}


PYBIND11_MODULE(libngsolve, m) {
  m.attr("__name__") = "ngsolve";
  ExportNgsolve(m);
}


#endif
