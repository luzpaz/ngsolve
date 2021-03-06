#ifndef FILE_THCURLFE_IMPL
#define FILE_THCURLFE_IMPL



namespace ngfem
{


  
  /*******************************************/
  /* T_HCurlHOFiniteElement                  */
  /*******************************************/

  
  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  CalcShape (const IntegrationPoint & ip, SliceMatrix<> shape) const
  {    
    TIP<DIM,AutoDiff<DIM>> tip = ip;
    this->T_CalcShape (tip, SBLambda ([shape](size_t i, auto s) 
                                           { FlatVec<DIM> (&shape(i,0)) = s.Value(); }));
  }

  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET, SHAPES,BASE> :: 
  CalcCurlShape (const IntegrationPoint & ip, SliceMatrix<> shape) const
  {  
    TIP<DIM,AutoDiff<DIM>> tip = ip;    
    this->T_CalcShape (tip, SBLambda ([shape](size_t i, auto s) 
                                           { FlatVec<DIM_CURL_(DIM)> (&shape(i,0)) = s.CurlValue(); }));
  } 

#ifndef FASTCOMPILE
  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET, SHAPES, BASE> :: 
  CalcMappedShape (const BaseMappedIntegrationPoint & bmip,
                   SliceMatrix<> shape) const
  {
    auto & mip = static_cast<const MappedIntegrationPoint<DIM,DIM>&> (bmip);
    Vec<DIM, AutoDiff<DIM> > adp = mip;
    TIP<DIM,AutoDiff<DIM>> tip(adp);
    this->T_CalcShape (tip, // GetTIP(mip),
                       SBLambda ([shape](size_t i, auto s) 
                                 { 
                                   FlatVec<DIM> (&shape(i,0)) = s.Value(); 
                                 }));
  }

  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  CalcMappedShape (const MappedIntegrationRule<DIM,DIM> & mir, 
                   SliceMatrix<> shape) const
  {
    for (size_t i = 0; i < mir.Size(); i++)
      CalcMappedShape (mir[i], shape.Cols(i*DIM,(i+1)*DIM));
  }

  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  CalcMappedShape (const SIMD_BaseMappedIntegrationRule & bmir, 
                   BareSliceMatrix<SIMD<double>> shapes) const
  {
    Iterate<4-DIM>
      ([this,&bmir,shapes](auto CODIM)
       {
         constexpr int DIMSPACE = DIM+CODIM.value;
         if (bmir.DimSpace() == DIMSPACE)
           {
             auto & mir = static_cast<const SIMD_MappedIntegrationRule<DIM,DIMSPACE>&> (bmir);
             for (size_t i = 0; i < mir.Size(); i++)
               {
                 auto shapei = shapes.Col(i);
                 this->T_CalcShape
                   (GetTIP(mir[i]),
                    SBLambda ([shapei,DIMSPACE] (size_t j, auto s)
                              {
                                auto shape = s.Value();
                                for (size_t k = 0; k < DIMSPACE; k++)
                                  shapei(j*DIMSPACE+k) = shape(k);
                              }));
               }
             
           }
       });
  }


  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  CalcMappedCurlShape (const BaseMappedIntegrationPoint & bmip,
                       SliceMatrix<> curlshape) const
  {
    auto & mip = static_cast<const MappedIntegrationPoint<DIM,DIM>&> (bmip);
    /*
    if (DIM == 2)
      {
        CalcCurlShape (mip.IP(), curlshape);
        curlshape /= mip.GetJacobiDet();        
      }
    else
    */
    {
      Vec<DIM, AutoDiff<DIM> > adp = mip;
      TIP<DIM,AutoDiff<DIM>> tip(adp);
      this->T_CalcShape (tip, // GetTIP(mip),
                         SBLambda ([&](size_t i, auto s) 
                                   { 
                                     FlatVec<DIM_CURL_(DIM)> (&curlshape(i,0)) = s.CurlValue(); 
                                   }));
    }
  }


  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  CalcMappedCurlShape (const MappedIntegrationRule<DIM,DIM> & mir, 
                       SliceMatrix<> curlshape) const
  {
    for (int i = 0; i < mir.Size(); i++)
      CalcMappedCurlShape (mir[i], 
                           curlshape.Cols(DIM_CURL_(DIM)*i, DIM_CURL_(DIM)*(i+1)));
  }    


  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  CalcMappedCurlShape (const SIMD_BaseMappedIntegrationRule & bmir, 
                       BareSliceMatrix<SIMD<double>> shapes) const
  {
    Iterate<4-DIM>
      ([this,&bmir,shapes](auto CODIM)
       {
         constexpr int DIMSPACE = DIM+CODIM.value;
         if (bmir.DimSpace() == DIMSPACE)
           {
             auto & mir = static_cast<const SIMD_MappedIntegrationRule<DIM,DIMSPACE>&> (bmir);
             constexpr int DIM_CURL = DIM_CURL_(DIMSPACE);        
             for (size_t i = 0; i < mir.Size(); i++)
               {
                 auto shapei = shapes.Col(i);
                 this->T_CalcShape (GetTIP(mir[i]),
                                    SBLambda ([shapei,DIM_CURL] (size_t j, auto s)
                                              {
                                                auto cshape = s.CurlValue();
                                                for (size_t k = 0; k < DIM_CURL; k++)
                                                  shapei(j*DIM_CURL+k) = cshape(k);
                                              }));
               }
           }
       });
  }

  

  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  auto T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  EvaluateCurlShape (const IntegrationPoint & ip, 
                     FlatVector<double> x,
                     LocalHeap & lh) const -> Vec<DIM_CURL_(DIM)>
  {
    Vec<DIM, AutoDiff<DIM> > adp = ip;
    TIP<DIM,AutoDiff<DIM>> tip(adp);
    
    Vec<DIM_CURL_(DIM)> sum = 0.0;
    this->T_CalcShape (tip, SBLambda ([&sum, x](size_t i, auto s) 
                                      { sum += x(i) * s.CurlValue(); }));
    return sum;
  }

  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  EvaluateCurl (const IntegrationRule & ir, FlatVector<> coefs, FlatMatrixFixWidth<DIM_CURL_(DIM)> curl) const
  {
    LocalHeapMem<10000> lhdummy("evalcurl-heap");
    for (int i = 0; i < ir.Size(); i++)
      curl.Row(i) = EvaluateCurlShape (ir[i], coefs, lhdummy);
  }

  
  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  Evaluate (const SIMD_BaseMappedIntegrationRule & bmir, BareSliceVector<> coefs,
            BareSliceMatrix<SIMD<double>> values) const
  {
    Iterate<4-DIM>
      ([this,&bmir,coefs,values](auto CODIM)
       {
         constexpr int DIMSPACE = DIM+CODIM.value;
         if (bmir.DimSpace() == DIMSPACE)
           {
             auto & mir = static_cast<const SIMD_MappedIntegrationRule<DIM,DIMSPACE>&> (bmir);
             for (size_t i = 0; i < mir.Size(); i++)
               {
                 Vec<DIMSPACE,SIMD<double>> sum(0.0);
                 this->T_CalcShape (GetTIP(mir[i]),
                                    SBLambda ([&sum,coefs] (size_t j, auto shape)
                                              {
                                                sum += coefs(j) * shape.Value();
                                              }));
                 for (size_t k = 0; k < DIMSPACE; k++)
                   values(k,i) = sum(k); 
               }
           }
       });
  }

  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  Evaluate (const SIMD_BaseMappedIntegrationRule & bmir, BareSliceVector<Complex> coefs,
            BareSliceMatrix<SIMD<Complex>> values) const
  {
    Iterate<4-DIM>
      ([this,&bmir,coefs,values](auto CODIM)
       {
         constexpr int DIMSPACE = DIM+CODIM.value;
         if (bmir.DimSpace() == DIMSPACE)
           {
             auto & mir = static_cast<const SIMD_MappedIntegrationRule<DIM,DIMSPACE>&> (bmir);
             for (size_t i = 0; i < mir.Size(); i++)
               {
                 Vec<DIMSPACE,SIMD<Complex>> sum = SIMD<Complex>(0.0);
                 this->T_CalcShape (GetTIP(mir[i]),
                                    SBLambda ([&sum,coefs] (size_t j, auto shape)
                                              {
                                                sum += coefs(j) * shape.Value();
                                              }));
                 for (size_t k = 0; k < DIMSPACE; k++)
                   values(k,i) = sum(k);
               }
           }
       });
  }

  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  EvaluateCurl (const SIMD_BaseMappedIntegrationRule & bmir, BareSliceVector<> coefs, BareSliceMatrix<SIMD<double>> values) const
  {
    Iterate<4-DIM>
      ([this,&bmir,coefs,values](auto CODIM)
       {
         constexpr int DIMSPACE = DIM+CODIM.value;
         if (bmir.DimSpace() == DIMSPACE)
           {
             constexpr int DIM_CURL = DIM_CURL_(DIMSPACE);
             auto & mir = static_cast<const SIMD_MappedIntegrationRule<DIM,DIMSPACE>&> (bmir);
             for (size_t i = 0; i < mir.Size(); i++)
               {
                 Vec<DIM_CURL,SIMD<double>> sum(0.0);            
                 this->T_CalcShape (GetTIP(mir[i]),
                                    SBLambda ([coefs,&sum] (size_t j, auto shape)
                                              {
                                                sum += coefs(j) * shape.CurlValue();
                                              }));
                 for (size_t k = 0; k < DIM_CURL; k++)
                   values(k,i) = sum(k).Data();
               }
           }
       });
  }

  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  EvaluateCurl (const SIMD_BaseMappedIntegrationRule & bmir, BareSliceVector<Complex> coefs, BareSliceMatrix<SIMD<Complex>> values) const
  {
    Iterate<4-DIM>
      ([this,&bmir,coefs,values](auto CODIM)
       {
         constexpr int DIMSPACE = DIM+CODIM.value;
         if (bmir.DimSpace() == DIMSPACE)
           {
             constexpr int DIM_CURL = DIM_CURL_(DIMSPACE);
             auto & mir = static_cast<const SIMD_MappedIntegrationRule<DIM,DIMSPACE>&> (bmir);
             
             for (size_t i = 0; i < mir.Size(); i++)
               {
                 Vec<DIM_CURL,SIMD<Complex>> sum = SIMD<Complex>(0.0);
                 this->T_CalcShape (GetTIP(mir[i]),
                                    SBLambda ([coefs, &sum] (size_t j, auto shape)
                                              {
                                                sum += coefs(j) * shape.CurlValue();
                                              }));
                 for (size_t k = 0; k < DIM_CURL; k++)
                   values(k,i) = sum(k);
               }
           }
       });
  }

  
  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  AddTrans (const SIMD_BaseMappedIntegrationRule & bmir, BareSliceMatrix<SIMD<double>> values,
            BareSliceVector<> coefs) const
  {
    Iterate<4-DIM>
      ([this,&bmir,coefs,values](auto CODIM)
       {
         constexpr int DIMSPACE = DIM+CODIM.value;
         if (bmir.DimSpace() == DIMSPACE)
           {
             auto & mir = static_cast<const SIMD_MappedIntegrationRule<DIM,DIMSPACE>&> (bmir);
             for (size_t i = 0; i < mir.Size(); i++)
               {
                 Vec<DIMSPACE,SIMD<double>> vali = values.Col(i);
                 this->T_CalcShape (GetTIP(mir[i]),
                                    SBLambda ([vali,coefs] (size_t j, auto s)
                                              {
                                                /*
                                                  auto shape = s.Value();
                                                  SIMD<double> sum = 0.0;
                                                   for (size_t k = 0; k < shape.Size(); k++)
                                                   sum += shape(k) * vali(k);
                                                   coefs(j) += HSum(sum);
                                                */
                                                coefs(j) += HSum(InnerProduct(s.Value(), vali));
                                              }));
               }
           }
       });
  }

  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  AddTrans (const SIMD_BaseMappedIntegrationRule & bmir, BareSliceMatrix<SIMD<Complex>> values,
            BareSliceVector<Complex> coefs) const
  {
    Iterate<4-DIM>
      ([this,&bmir,coefs,values](auto CODIM)
       {
         constexpr int DIMSPACE = DIM+CODIM.value;
         if (bmir.DimSpace() == DIMSPACE)
           {
             auto & mir = static_cast<const SIMD_MappedIntegrationRule<DIM,DIMSPACE>&> (bmir);
             for (size_t i = 0; i < mir.Size(); i++)
               {
                 Vec<DIMSPACE,SIMD<Complex>> vali = values.Col(i);
                 this->T_CalcShape (GetTIP(mir[i]),
                                    SBLambda ([vali,coefs] (size_t j, auto s)
                                              {
                                                /*
                                                  auto shape = s.Value();
                                                  SIMD<Complex> sum = 0.0;
                                                  for (size_t k = 0; k < shape.Size(); k++)
                                                     sum += shape(k) * vali(k);
                                                     coefs(j) += HSum(sum);
                                                */
                                                coefs(j) += HSum(InnerProduct(s.Value(), vali));
                                              }));
               }
           }
       });
  }

  
  
  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  AddCurlTrans (const SIMD_BaseMappedIntegrationRule & bmir, BareSliceMatrix<SIMD<double>> values,
                BareSliceVector<> coefs) const
  {
    Iterate<4-DIM>
      ([this,&bmir,coefs,values](auto CODIM)
       {
         constexpr int DIMSPACE = DIM+CODIM.value;
         constexpr int DIM_CURL = DIM_CURL_(DIMSPACE);                                         
         if (bmir.DimSpace() == DIMSPACE)
           {
             auto & mir = static_cast<const SIMD_MappedIntegrationRule<DIM,DIMSPACE>&> (bmir);
             for (size_t i = 0; i < mir.Size(); i++)
               {
                 Vec<DIM_CURL,SIMD<double>> vali = values.Col(i);
                 this->T_CalcShape (GetTIP(mir[i]),
                                    SBLambda ([vali,coefs] (size_t j, auto s)
                                              {
                                                /*
                                                  auto cshape = s.CurlValue();
                                                  SIMD<double> sum = 0.0;
                                                  for (size_t k = 0; k < cshape.Size(); k++)
                                                  sum += cshape(k) * vali(k);
                                                  coefs(j) += HSum(sum);
                                                */
                                                coefs(j) += HSum(InnerProduct(s.CurlValue(), vali));
                                              }));
               }
           }
       });
  }
  
  
  template <ELEMENT_TYPE ET, typename SHAPES, typename BASE>
  void T_HCurlHighOrderFiniteElement<ET,SHAPES,BASE> :: 
  AddCurlTrans (const SIMD_BaseMappedIntegrationRule & bmir, BareSliceMatrix<SIMD<Complex>> values,
                BareSliceVector<Complex> coefs) const
  {
    Iterate<4-DIM>
      ([this,&bmir,coefs,values](auto CODIM)
       {
         constexpr int DIMSPACE = DIM+CODIM.value;
         constexpr int DIM_CURL = DIM_CURL_(DIMSPACE);                                         
         if (bmir.DimSpace() == DIMSPACE)
           {
             auto & mir = static_cast<const SIMD_MappedIntegrationRule<DIM,DIMSPACE>&> (bmir);
             for (size_t i = 0; i < mir.Size(); i++)
               {
                 Vec<DIM_CURL,SIMD<Complex>> vali = values.Col(i);
                 this->T_CalcShape (GetTIP(mir[i]),
                                    SBLambda ([vali,coefs] (size_t j, auto s)
                                              {
                                                /*
                                                  auto cshape = s.CurlValue();
                                                  SIMD<Complex> sum = 0.0;
                                                  for (size_t k = 0; k < cshape.Size(); k++)
                                                  sum += cshape(k) * vali(k);
                                                  coefs(j) += HSum(sum);
                                                   */
                                                coefs(j) += HSum(InnerProduct(s.CurlValue(), vali));
                                              }));
               }
           }
       });
  }

#endif
}

#endif
