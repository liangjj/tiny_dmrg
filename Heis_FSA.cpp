/** 
 * @file Heis_FSA.cpp
 * @brief The main c++ file for the DMRG
 * 
 * @mainpage
 * @author Roger Melko 
 * @author Ivan Gonzalez
 * @date February 9th, 2011
 * 
 * @brief Elementry DMRG simulation for the Heisenberg chain; 
 *  \f$H= \sum_{ij} (S^x_i S^x_j + S^y_i S^y_j +  S^z_i S^z_j ) \f$
 * 
 *  - Uses a "symmetric" infinite system algorithm to build chain 
 *  - Exact diagonalization performed with Lanczos
 *  - finite system sweep - symmetric build of L and R blocks
 */
#include "blitz/array.h"
#include "block.h"
#include "matrixManipulation.h"
#include "lanczosDMRG.h"
#include "HHtridi8.h"

BZ_USING_NAMESPACE(blitz)

int main()
{
    int iter, NumI;
    int i1, i1p, i2, i2p, i3, i4; 
    int b1; 
    int m, st;     //# states
    int truncflag;
    int sites, NumS;     //# sites (SYSTEM)
    int Esites;          //# sites (ENVIRONMENT)
    int FSAend;

    /** \var BLOCK blkS \brief create the system block */
    BLOCK blkS;  //the system block
    ///create the environment block
    BLOCK blkE;  //the environment block

    cout<<"# states to keep: ";
    cin>>m;
    cout<<"System size : ";
    cin>>NumS;
    cout<<"FSA sweeps : ";
    cin>>NumI;

    //Matrices
    Array<double,4> TSR(2,2,2,2);  //tensor product for Hab hamiltonian

    Array<double,4> Habcd(4,4,4,4); //superblock hamiltonian
    Array<double,2> Psi(4,4); // ground state wavefunction
    Array<double,2> rhoTSR(4,4); // reduced density matrix
    Array<double,2> OO(m,4);   //the TRUNCATION matrix
    Array<double,2> OT(4,m);   // trasposed truncation matrix
    Array<double,2> Hl(4,m);   // the left half of new system H

    Array<double,2> HAp;  //A' block hamiltonian
    Array<double,2> SzB;   //Sz(left) operator  
    Array<double,2> SmB;   //Sm(left) operator
    Array<double,2> SpB;   //Sp(left) operator

    //create identity matrices
    Array<double,2> I2(2,2), I4(4,4), I2m(2*m,2*m);   
    I2=0.0; I4=0.0; I2m=0.0;
    for (b1=0; b1<2; b1++) I2(b1,b1)=1.0;
    for (b1=0; b1<4; b1++) I4(b1,b1)=1.0;
    for (b1=0; b1<(2*m); b1++) I2m(b1,b1)=1.0;
    Array<double,2> I2st(4,4);
    I2st = I4;

    // Create pauli matrices
    Array<double,2> Sz(2,2), Sp(2,2), Sm(2,2);
    Sz = 0.5, 0,
       0, -0.5;
    Sp = 0, 1.0,
       0, 0;
    Sm = 0, 0,
       1.0, 0;

    //tensor indices
    firstIndex i;    secondIndex j; 
    thirdIndex k;    fourthIndex l; 

    //create a tensor product: two-site Hamiltonian
    TSR = Sz(i,k)*Sz(j,l)+ 0.5*Sp(i,k)*Sm(j,l) + 0.5*Sm(i,k)*Sp(j,l) ;
    //write as 2D matrix in combined basis
    Array<double,2> H12 = reduceM2M2(TSR,2,2);
    //cout<<"H12 "<<H12<<endl;

    TSR = Sz(i,k)*I2(j,l);
    Array<double,2> SzAB = reduceM2M2(TSR,2,2);

    TSR = Sm(i,k)*I2(j,l);
    Array<double,2> SmAB = reduceM2M2(TSR,2,2);

    TSR = Sp(i,k)*I2(j,l);
    Array<double,2> SpAB = reduceM2M2(TSR,2,2);

    blkS.HAB.resize(4,4);
    blkS.HAB = H12;
    st = 2;     //start with a 2^2=4 state system
    sites = 2;  //in SYSTEM block


    /******infinite system algorithm loop   ***********************/
    /******build system until number of desired states m **********/

    truncflag = 0;

    while (sites <= (NumS)/2 ) 
    {
	Habcd = blkS.HAB(i,k)*I2st(j,l)+ 
	    I2st(i,k)*blkS.HAB(j,l)+
	    SzAB(i,k)*SzAB(j,l)+0.5*SpAB(i,k)*SmAB(j,l)+0.5*SmAB(i,k)*SpAB(j,l);
      
	double Eval=calculateGroundState(Habcd,Psi,(4*st*st));
	
//      cout<<"sites: "<<2.0*sites;
//      if (truncflag == 0) cout<<" e ";
//      else cout<<" t ";
	cout<<"# "<<2.0*sites<<" "<<1.0/(2.0*sites);
	cout<<" "<<Eval/(2.0*sites)<<endl;

	rhoTSR=calculateReducedDensityMatrix(Psi);
	
	if (2*st <= m){     // NO TRUNCATION
	  
	    OO.resize(2*st,2*st);
	    OT.resize(2*st,2*st);
	    DMlargeEigen(rhoTSR, OO, 2*st, 2*st);   
	    for (i1=0; i1<2*st; i1++)
	    for (i2=0; i2< 2*st; i2++)
	      OT(i1,i2) = OO(i2,i1); 
	    Hl.resize(2*st,2*st);
	    HAp.resize(2*st,2*st);
	    SzB.resize(2*st,2*st);
	    SpB.resize(2*st,2*st);
	    SmB.resize(2*st,2*st);
	    st *= 2;
	}
	else {            // TRUNCATION
	    if (truncflag == 0 || truncflag == 3){
		OO.resize(m,2*st);
		OT.resize(2*st,m);
		Hl.resize(2*st,m);
		truncflag ++; // 1 or 4
	    }
	    DMlargeEigen(rhoTSR, OO, 2*st, m);   
	    for (i1=0; i1<2*st; i1++)
		for (i2=0; i2< m; i2++)
		    OT(i1,i2) = OO(i2,i1);
	}
	
	if (truncflag < 2){
	  if (truncflag == 1) {
	    truncflag = 2;	
	    st = m;
	    HAp.resize(m,m);
	    SzB.resize(m,m);
	    SpB.resize(m,m);
	    SmB.resize(m,m);
	  }
	  TSR.resize(st,2,st,2);
	}

    //transform Operator Matrices to new basis
    Hl = sum(blkS.HAB(i,k)*OT(k,j),k);   //Ha'
    HAp = sum(OO(i,k)*Hl(k,j),k);   //(inner product)

    Hl = sum(SzAB(i,k)*OT(k,j),k);  
    SzB = sum(OO(i,k)*Hl(k,j),k);    

    Hl = sum(SpAB(i,k)*OT(k,j),k);  
    SpB = sum(OO(i,k)*Hl(k,j),k);

    Hl = sum(SmAB(i,k)*OT(k,j),k);  
    SmB = sum(OO(i,k)*Hl(k,j),k);

    TSR = HAp(i,k)*I2(j,l) + SzB(i,k)*Sz(j,l)+ 
      0.5*SpB(i,k)*Sm(j,l) + 0.5*SmB(i,k)*Sp(j,l) ;
    blkS.HAB.resize(2*st,2*st);            //Hamiltonian for next iteration
    blkS.HAB = reduceM2M2(TSR,st,2);
    //   cout<<HAB<<endl;
    
    if (truncflag < 3){
      if  (truncflag == 2) {
	truncflag = 3;
	I2st.resize(2*m,2*m);    //redifine identity matrix
	I2st = I2m;
      }
      else{
	I2st.resize(4*st,4*st);    //redifine I (still growing)
	I2st = 0.0;
	for (b1=0; b1<(4*st); b1++) I2st(b1,b1)=1.0;
      }
      SzAB.resize(2*st,2*st);  //Operators for next iteration
      TSR = I2st(i,k)*Sz(j,l);
      SzAB = reduceM2M2(TSR,st,2);
      
      SpAB.resize(2*st,2*st);
      TSR = I2st(i,k)*Sp(j,l);
      SpAB = reduceM2M2(TSR,st,2);
      
      SmAB.resize(2*st,2*st);
      TSR = I2st(i,k)*Sm(j,l);
      SmAB = reduceM2M2(TSR,st,2);        
      
      Habcd.resize(2*st,2*st,2*st,2*st);   //re-prepare superblock matrix
      Psi.resize(2*st,2*st);             //GS wavefunction
      rhoTSR.resize(2*st,2*st);
    }

    sites++;

    blkS.size = sites;  //this is the size of the system block
    blkS.ISAwrite(sites);
    
    }//end INFINITE SYSTEM ALGORITHM iteration

  
    std::cout<<"# End ISA; sites = "<<sites<<std::endl;


    /******FINITE system algorithm loop   ***********************/  

    //find maximum extent of FSA sweep
    for (FSAend = 3; FSAend < NumS; FSAend++)
	if (powf(2.0,FSAend) >= 2.0*m) break;
  
    //sites= FSAend;
    sites = NumS/2;
    blkS.FSAread(sites,1);

  
    for (iter=0; iter<NumI; iter++){
    
	while (sites <= NumS-FSAend){

	    //std::cout<<sites<<" before "<<blkE.HAB;
	    Esites = NumS - sites;
	    blkE.FSAread(Esites,iter);

	    Habcd = blkE.HAB(i,k)*I2st(j,l)+I2st(i,k)*blkS.HAB(j,l) +
	    SzAB(i,k)*SzAB(j,l)+0.5*SpAB(i,k)*SmAB(j,l)+0.5*SmAB(i,k)*SpAB(j,l);

	    double Eval=calculateGroundState(Habcd,Psi,(4*m*m));

	    if (iter%2 == 0) cout<<sites<<" "<<Esites;
	    else cout<<Esites<<" "<<sites;
	    cout<<" "<<Eval/(Esites+sites)<<endl;
	  
	    rhoTSR=calculateReducedDensityMatrix(Psi);
     
	    DMlargeEigen(rhoTSR, OO, 2*m, m);   
	    for (i1=0; i1<2*m; i1++)
	      for (i2=0; i2< m; i2++)
		OT(i1,i2) = OO(i2,i1);
      
	    //transform Operator Matrices to new basis
	    Hl = sum(blkS.HAB(i,k)*OT(k,j),k);   //Ha'
	    HAp = sum(OO(i,k)*Hl(k,j),k);   //(inner product)

	    Hl = sum(SzAB(i,k)*OT(k,j),k);  
	    SzB = sum(OO(i,k)*Hl(k,j),k);    

	    Hl = sum(SpAB(i,k)*OT(k,j),k);  
	    SpB = sum(OO(i,k)*Hl(k,j),k);

	    Hl = sum(SmAB(i,k)*OT(k,j),k);  
	    SmB = sum(OO(i,k)*Hl(k,j),k);

	    //Add spin to the system block only
	    TSR = HAp(i,k)*I2(j,l) + SzB(i,k)*Sz(j,l)+ 
	    0.5*SpB(i,k)*Sm(j,l) + 0.5*SmB(i,k)*Sp(j,l);       
	    blkS.HAB = reduceM2M2(TSR,m,2);
  
	    sites++;

	    blkS.size = sites;
	    blkS.FSAwrite(sites,iter);
	}//while

	//cout<<"end "<<blkS.size<<" "<<sites<<endl;
	sites = FSAend;
	blkS.FSAread(sites,iter);

	//sites++;
  }//Iter
  return 0;
} // end main
