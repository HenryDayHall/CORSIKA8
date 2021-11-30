namespace corsika {
  inline bool Box::contains(Point const& p) const {
    if ((abs(p.getX(cs_)) < x_) && (abs(p.getY(cs_)) < y_) && (abs(p.getZ(cs_)) < z_))
      return true;
    else
      return false;
  }

  inline std::string Box::asString() const {
    std::ostringstream txt;
    txt << "center=" << center_ << ", x-axis=" << DirectionVector{cs_, {1, 0, 0}}
        << ", y-axis: " << DirectionVector{cs_, {0, 1, 0}}
        << ", z-axis: " << DirectionVector{cs_, {0, 0, 1}};
    return txt.str();
  }

  template <typename TDim>
  inline void Box::rotate(QuantityVector<TDim> const& axis, double const angle) {
    cs_ = make_rotation(cs_, axis, angle);
  }

} // namespace corsika