#include <cmath>
#include <fstream>

#include <iostream>
#include <functional>
#include <unordered_map>

namespace memoization {

  // https://stackoverflow.com/questions/24637619/memoization-functor-wrapper-in-c
  template<typename Sig, typename F=Sig* >
  struct memoize_t;
  template<typename R, typename Arg, typename F>
  struct memoize_t<R(Arg), F> {
	  F f;
	  mutable std::unordered_map< Arg, R > results;
	  R operator()( Arg a ) const {
		  auto it = results.find(a);
		  if (it != results.end())
			  return it->second;
		  else
			  return results[a] = f(a);
	  }
  };

  template<typename F>
  memoize_t<F> memoize( F* func ) {
	  return {func};
  }

  /* // Demo:
  int foo(int x) {
    static auto mem = memoize(foo);
    auto&& foo = mem;
    std::cout << "processing...\n";
    if (x <= 0) return foo(x+2)-foo(x+1); // bwahaha
    if (x <= 2) return 1;
    return foo(x-1) + foo(x-2);;
  }
  int main() {
    std::cout << foo(10) << "\n";
  }
  */

  
  // Disambiguate the types with this wrapper:
  double sqrt(double d) {
    return ::sqrt(d);
  }
  static auto sqrt_memoization = memoize(sqrt);
  double sqrt_memoize(double d) {
    return sqrt_memoization(d);
  }

  
  struct PersistentMemoization {
    ~PersistentMemoization() {
      printf("PersistentMemoization destructor called, writing memoizations out to files...\n");
      
      std::ofstream sqrt_memoization_file("sqrt_memoization.csv", std::ios::out | std::ios::trunc);
      for(auto it = sqrt_memoization.results.begin(); it != sqrt_memoization.results.end(); ++it) {
	sqrt_memoization_file << std::hexfloat << it->first << "," << it->second << '\n';
      }
      
      printf("PersistentMemoization destructor finished\n");
    }
  };
  //static PersistentMemoization dtorActivatedPersistentMemoization;
}
