#ifndef SLT_RECT_PACK_H
#define SLT_RECT_PACK_H

#include <array>
#include <limits>
#include <vector>
#include <cassert>
#include <optional>
#include <tuple>

namespace slt {

  // Default scorer: fits the rect as snuggly as possible in a dimension
  // Note: lower means better.
  template<typename T>
  T DefaultRectScorer(std::array<T, 2> const& src, std::array<T, 2> const& dst) {
    return std::max(dst[0] - src[0], dst[1] - src[1]);
  }

  // This is an open-ended incremental rect packer.
  //
  // For optimal results, you should sort the list of rectangles to pack
  // beforehand, but this is not a hard requirement.
  //
  // If using a custom allocator, it has to be rebindable.
  template<typename T, typename Alloc = std::allocator<char>>
  class RectPacker2D {
  public:
    using Vec = std::array<T, 2>;
    struct Result {
      Vec pos;
      bool flipped;
    };

    explicit RectPacker2D(Vec bin_size, Alloc const& alloc = Alloc())
      : free_rects_(alloc) {
      free_rects_.emplace_back(FreeRect{ {T(0), T(0)}, bin_size });
    }

    // Tries to pack a rect of size `rect`.
    template<typename RectScorer = decltype(DefaultRectScorer<T>)>
    std::optional<Result> pack(Vec rect, RectScorer scorer = DefaultRectScorer<T>) {
      //1. Chose where to store the rect
      FreeRectIte chosen_slot; 
      bool flipped;
      std::tie(chosen_slot, flipped) = chooseSlot(rect, scorer);

      if (chosen_slot == free_rects_.end()) {
        return std::nullopt;
      }
      FreeRect destination = *chosen_slot;

      //2. remove the chosen rect from the free list
      removeFreeRect(chosen_slot);
      //N.B. chosen_slot is an invalid iterator past this spot

      //3. add the remainders to the free list
      const T rem_w = destination.size[0] - rect[0];
      const T rem_h = destination.size[1] - rect[1];

      bool right_added = addFreeRect(
        { destination.pos[0] + rect[0], destination.pos[1] },
        { rem_w, destination.size[1] }
      );

      bool bottom_added = addFreeRect(
        { destination.pos[0], destination.pos[1] + rect[1] },
        { destination.size[0], rem_h }
      );
      
      //If both insertions were successful, bind them together.
      if (right_added && bottom_added) {
        std::prev(free_rects_.end())->counterpart = free_rects_.size() - 2;
        std::prev(free_rects_.end(), 2)->counterpart = free_rects_.size() - 1;
      }

      return Result{
        {destination.pos[0], destination.pos[1]},
        flipped
      };
    }

    // If calling this, you are responsible for ensure it does not overlap with any
    // other existing free rect. returns wether insertion was successfull.
    bool addFreeRect(Vec const& pos, Vec const& size) {
      if (size[0] > T(0) && size[1] > T(0)) {
        free_rects_.emplace_back(FreeRect{ pos, size });
        return true;
      }
      return false;
    }

  private:
    struct FreeRect {
      Vec pos;
      Vec size;

      // Two rects with bi-directional linking counterpart have an overlapping section
      // following this pattern.
      // XXXAAA
      // XXXAAA
      // BBB***
      // (the * part belongs to both A and B).
      // Whichever rect, A or B, is used first will claim the shared section as its own
      // and it's counterpart will be shrunk accordingly.
      typename std::vector<FreeRect>::difference_type counterpart = -1;
    };

    //using FreeRectAllocator = typename Alloc::template rebind<FreeRect>::other;
    using FreeRectAllocator = typename std::allocator_traits<Alloc>::template rebind_alloc<FreeRect>;
    using FreeRectCollection = std::vector<FreeRect, FreeRectAllocator>;
    using FreeRectIte = typename FreeRectCollection::iterator;

    template<typename RectScorer>
    std::tuple<FreeRectIte, bool> chooseSlot(Vec const& rect, RectScorer scorer) {
      const T non_fit_score = std::numeric_limits<T>::max();

      Vec flipped_rect = { rect[1], rect[0] };

      // 1. Identify where we will store the rect.
      T best_score = non_fit_score;
      auto best_rect = free_rects_.end();
      bool flip = false;

      for (auto ite = free_rects_.begin(); ite != free_rects_.end(); ++ite) {
        // Test for perfect fit
        if (rect == ite->size) {
          best_rect = ite;
          flip = false;
          break;
        }

        if (flipped_rect == ite->size) {
          best_rect = ite;
          flip = true;
          break;
        }

        T score = fits(rect, ite->size) ? scorer(rect, ite->size) : non_fit_score;
        T flipped_score = fits(flipped_rect, ite->size) ? scorer(flipped_rect, ite->size) : non_fit_score;

        if (score < best_score) {
          best_rect = ite;
          best_score = score;
          flip = false;
        }

        if (flipped_score < best_score) {
          best_rect = ite;
          best_score = score;
          flip = true;
        }
      }

      return std::make_tuple(best_rect, flip);
    }

    void removeFreeRect(FreeRectIte target) {
      if (target->counterpart != -1) {
        // If the chosen rect had a counterpart, shrink it.
        auto cp = &free_rects_[target->counterpart];
        assert(cp->counterpart == target - free_rects_.begin());

        if (cp->pos[0] < target->pos[0]) {
          //cp was the bottom one
          cp->size[0] = target->pos[0] - cp->pos[0];
        }
        else {
          //cp was the top one
          assert(cp->pos[1] < target->pos[1]);
          cp->size[1] = target->pos[1] - cp->pos[1];
        }
        cp->counterpart = -1;
      }

      auto last = std::prev(free_rects_.end());
      if (last != target) {
        *target = std::move(*last);
        if (target->counterpart != -1) {
          auto cp = &free_rects_[target->counterpart];
          cp->counterpart = target - free_rects_.begin();
        }
      }
      free_rects_.pop_back();
    }

    static bool fits(Vec src, Vec dst) {
      return dst[0] >= src[0] && dst[1] >= src[1];
    }

    FreeRectCollection free_rects_;

  };
}
#endif