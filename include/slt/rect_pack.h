#ifndef SLT_RECT_PACK_H
#define SLT_RECT_PACK_H

#include <vector>

namespace slt {
  // This is an open-ended incremental rect packer.
  //
  // For optimial results, you should sort the list of rectangles to pack
  // before hand, but this is not a hard requirement. If using std::sort, 
  // you should use std::greater<> as the comparator. 
  //
  template<typename T>
  class RectPacker2D {
  public:
    struct RectSize {
      T width;
      T height;

      inline bool operator>(RectSize const& rhs) {
        return (h==rhs.h) ? w > rhs.w : h > rhs.h;
      }
    };

    struct Result {
      T x;
      T y;
      T width;
      T height;
    };

    inline RectPacker2D(RectSize bin_size);

    inline Result pack(RectSize rect);
 

  private:
    inline T getScore(RectSize const& src, RectSize const& dst) const;

    struct FreeRect {
      FreeRect(T x_, T y_, T w, T h)
        : x(x_), y(y_), size{ w, h } {}

      T x;
      T y;
      RectSize size;
    };

    std::vector<FreeRect> free_rects_;
  };

  template<typename T>
  RectPacker2D<T>::RectPacker2D(RectSize bin_size) {
    free_rects_.emplace_back(FreeRect{T(0), T(0), bin_size.width, bin_size.height});
  }

  template<typename T>
  typename RectPacker2D<T>::Result RectPacker2D<T>::pack(RectSize rect) {

    RectSize flipped_rect = {
      rect.height,
      rect.width
    };

    // 1. Identify where we will store the rect.
    T best_score = std::numeric_limits<T>::max();
    std::vector<FreeRect>::iterator best_rect = free_rects_.end();
    bool flip = false;

    for(auto ite = free_rects_.begin(); ite != free_rects_.end(); ++ite) {
      T score = getScore(rect, ite->size);
      T flipped_score = getScore(flipped_rect, ite->size);
      
      if(score == T(0)) {
        // perfect fit!
        best_rect = ite;
        flip = false;
        break;
      }

      if(flipped_score == T(0)) {
        // perfect fit!
        best_rect = ite;
        flip = true;
        break;
      }

      if(score < best_score) {
        best_rect = ite;
        best_score = score;
        flip = false;
      }

      if(flipped_score < best_score) {
        best_rect = ite;
        best_score = score;
        flip = true;
      }
    }

    if(best_rect == free_rects_.end()) {
      return Result {T(0), T(0) ,T(0) ,T(0)};
    }

    if(flip) {
      std::swap(rect.width, rect.height);
    }

    //3. remove the chosen rect
    FreeRect destination = *best_rect;
  
    auto last = std::prev(free_rects_.end());
    if(last != best_rect) {
      *best_rect = std::move(*last);
    }
    free_rects_.pop_back();

    // Insert the remainders
    const T rem_w = destination.size.width - rect.width;
    const T rem_h = destination.size.height - rect.height;
    if (rem_h == T(0)) {
      if(rem_w > (0)) { 
        free_rects_.emplace_back(
          destination.x + rect.width,
          destination.y,
          rem_w,
          rect.height);
      }
    }
    else if (rem_w == 0) {
      //rem_h is implicitly > 0
      free_rects_.emplace_back(
        destination.x,
        destination.y + rect.height,
        rect.width,
        rem_h);
    }
    else {
      //rem_h > 0 and rem_w > 0
      bool horiz_split = rem_w < rem_h;
      if (horiz_split) {
        free_rects_.emplace_back(
          destination.x + rect.width,
          destination.y,
          rem_w,
          rect.height);

        free_rects_.emplace_back(
          destination.x,
          destination.y + rect.height,
          destination.size.width,
          rem_h);
      }
      else {
        //vertical split
        free_rects_.emplace_back(
          destination.x,
          destination.y + rect.height,
          rect.width,
          rem_h);

        free_rects_.emplace_back(
          destination.x + rect.width,
          destination.y,
          rem_w,
          destination.size.height);
      }
    }

    return Result{
      destination.x, 
      destination.y, 
      rect.width,
      rect.height
    };
  }

  template<typename T>
  T RectPacker2D<T>::getScore(RectSize const& src, RectSize const& dst) const {

    if (src.width > dst.width || src.height > dst.height) {
      // doesn't fit
      return std::numeric_limits<T>::max();
    }

    T rem_width = dst.width - src.width;
    T rem_height = dst.height - src.height;

    if (rem_width == T(0) && rem_height == T(0) ){
      // perfect fit
      return T(0);
    }

    return std::min(rem_width, rem_height);

  }

}

#endif