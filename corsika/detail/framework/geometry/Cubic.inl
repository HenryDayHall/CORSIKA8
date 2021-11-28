

namespace corsika {
  inline bool Cubic::contains(Point const& p) const {
    if ((abs(p.getX(cs_)) < x_) && (abs(p.getY(cs_)) < y_) && (abs(p.getZ(cs_)) < z_))
      return true;
    else
      return false;
  }

  inline std::string Cubic::asString() const {
    std::ostringstream txt;
    txt << "center=" << center_ << ", x-axis=" << DirectionVector{cs_, {1, 0, 0}}
        << ", y-axis: " << DirectionVector{cs_, {0, 1, 0}}
        << ", z-axis: " << DirectionVector{cs_, {0, 0, 1}};
    return txt.str();
  }

} // namespace corsika