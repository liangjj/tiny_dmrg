/**
 * @file tred3.cpp
 *
 * @brief Householder reduction for a real, symmetric matrix to a
 * tridiagonal form
 *
 * @author Roger Melko 
 * @author Ivan Gonzalez
 * @date $Date$
 *
 * $Revision$ 
 */

//set the below value to "1" if you want the eigenvectors, and to "0" if not
#define EVECTS 1  

#include <cmath>
#include "blitz/array.h"
#include "tred3.h"

/**
 *
 * @brief A function to reduce a real symmetric matrix a to tridiagonal form
 *
 * @param a the symmetric matrix you want to tridiagonalize
 * @param d an array with the elements in the diagonal of the matrix
 * @param e an array with the elements in the off-diagonal of the matrix
 * @param n an int with the size of the array d
 *
 * Implements the Householder reduction. On entrance d, and e can be
 * garbage.
 * On return, a is replaced by the orthogonal matrix effecting the 
 * transformation, the diagonal elements are stored in d, and the 
 * off-diagonal elements are stored in e
 */
void tred3(blitz::Array<double,2>& a, blitz::Array<double,1>& d, blitz::Array<double,1>& e, int n)
{
  int i,j,k,l;
  double f,g,h,hh,scale;
 
  for (i=n-1;i>0;i--)
    { l=i-1;h=scale=0.0;
      if (l>0)
        { for (k=0;k<=l;k++) scale+=fabs(a(i,k));
          if (scale==0.0) {e(i)=a(i,l);continue;}// .....skip transformation
          // .............................. use scaled a's for transformation
          for (k=0;k<=l;k++) { a(i,k)/=scale;h+=a(i,k)*a(i,k);}
          f=a(i,l);
          g=sqrt(h);
          if (f>0.0) g=-g;
          e(i)=scale*g;
          h-=f*g;
          a(i,l)=f-g;
          f=0.0;
          for (j=0;j<=l;j++)
            { a(j,i)=a(i,j)/h;g=0.0;
              for (k=0;k<=j;k++) g+=a(j,k)*a(i,k);
              for (k=j+1;k<=l;k++) g+=a(k,j)*a(i,k);
              e(j)=g/h;
              f += e(j)*a(i,j);
            }
          hh=f/(h+h);
          for (j=0;j<=l;j++)
            { f=a(i,j);e(j)=g=e(j)-hh*f;
              for (k=0;k<=j;k++) a(j,k)-=(f*e(k)+g*a(i,k));
            }
        }
      else e(i)=a(i,l);
      d(i)=h;
    }
                          // eigenvectors
#if (EVECTS == 1)
  d(0)=0.0;e(0)=0.0;
  for (i=0;i<n;i++)
    { l=i-1;
      if (d(i))
        { for (j=0;j<=l;j++)
            { g=0.0;
              for (k=0;k<=l;k++) g+=a(i,k)*a(k,j);
              for (k=0;k<=l;k++) a(k,j)-=g*a(k,i);
            }
        }
      d(i)=a(i,i);
      a(i,i)=1.0;
      for (j=0;j<=l;j++) a(j,i)=a(i,j)=0.0;
    }
#else 
  for (i=0;i<n;i++) d(i)=a(i,i);
#endif

  for (i=0;i<n-1;i++) e(i)=e(i+1);
  e(n-1)=0;
}
// end tred3.cpp
