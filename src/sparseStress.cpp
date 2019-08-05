#include <Rcpp.h>
using namespace Rcpp;


// [[Rcpp::export]]
NumericMatrix sparseStress(NumericMatrix y,
                           NumericMatrix D,
                           List Rp,
                           IntegerVector pivots,
                           List adjL) {

  int j;

  double diff  = 1;
  double tdiff = 0;
  double xerr;
  double yerr;
  double denom;
  int iter=0;
  double tx;
  double ty;

  //number of nodes and pivots
  int n = y.nrow();
  int m = pivots.length();

  //initialize coordinates
  NumericMatrix x(n,2);
  for(int i=0;i<n;++i){
    x(i,0)=y(i,0);
    x(i,1)=y(i,1);
  }


  NumericMatrix W(n,m);
  NumericVector wsum(n);

  //reweighting
  for(int i=0;i<n;++i){
    std::vector<int> Ni = as<std::vector<int> >(adjL[i]);
    for(int k=0;k<m;++k){
      int p = pivots[k];
      if(i!=p){
        double s=0;
        std::vector<int> Rpi = as<std::vector<int> >(Rp[k]);
        for(std::vector<int>::size_type l = 0; l!=Rpi.size(); ++l){
          j = Rpi[l];
          if(D(j,k)<=(D(i,k)/2)){
            s+=1;
          }
        }
        // Rcout << "The value is " << s << std::endl;
        // s = sum(D(_,k)<=(D(i,k)/2));
        W(i,k) = s/(D(i,k)*D(i,k));

        if(std::find(Ni.begin(), Ni.end(), p)==Ni.end()){
          wsum[i]+=W(i,k);
        }
      }
    }
  }

  while((diff > 0.0001) & (iter<500)){
    // Rcout << diff << std::endl;
    iter+=1;
    diff=0;
    for(int i=0;i<n;i++){
      std::vector<int> Ni = as<std::vector<int> >(adjL[i]);
      tx=0;
      ty=0;
      for(std::vector<int>::size_type k = 0; k!=Ni.size(); ++k){
        j = Ni[k];
        denom = sqrt((x(i,0)-x(j,0))*(x(i,0)-x(j,0))+(x(i,1)-x(j,1))*(x(i,1)-x(j,1)));
        if(denom>0.001){
          // xnew(i,0)+=x(j,0)+(x(i,0)-x(j,0))/denom;
          // xnew(i,1)+=x(j,1)+(x(i,1)-x(j,1))/denom;
          tx+=x(j,0)+(x(i,0)-x(j,0))/denom;
          ty+=x(j,1)+(x(i,1)-x(j,1))/denom;
        }
      }

      for(int p=0;p<m;++p){
        j=pivots[p];
        if(std::find(Ni.begin(), Ni.end(), j)==Ni.end()){
          denom = sqrt((x(i,0)-x(j,0))*(x(i,0)-x(j,0))+(x(i,1)-x(j,1))*(x(i,1)-x(j,1)));
          if(denom>0.001){
            // xnew(i,0)+=W(i,p)*(x(j,0)+(D(i,p)*(x(i,0)-x(j,0)))/denom);
            // xnew(i,1)+=W(i,p)*(x(j,1)+(D(i,p)*(x(i,1)-x(j,1)))/denom);
            tx+=W(i,p)*(x(j,0)+(D(i,p)*(x(i,0)-x(j,0)))/denom);
            ty+=W(i,p)*(x(j,1)+(D(i,p)*(x(i,1)-x(j,1)))/denom);
          }
        }
      }

      tx=tx/(wsum[i]+Ni.size());
      ty=ty/(wsum[i]+Ni.size());

      if((tx > 0.1) & (x(i,0)>0.1)){
        xerr = std::abs((tx-x(i,0))/x(i,0));
      } else{
        xerr = 0;
      }
      if((ty>0.1) & (x(i,1)>0.1)){
        yerr = std::abs((ty-x(i,1))/x(i,1));
      } else{
        yerr = 0;
      }
      tdiff = xerr + yerr;

      diff+= tdiff;
      x(i,0) = tx;
      x(i,1) = ty;

    }
    // Rcout << x(99,0) << " " << x(99,1) <<" "<< wsum[99]<< std::endl;
    // Rcout << "The diff is " << diff << std::endl;
  }

  return x;
}

