#include <process/null_model/NullModel.h>

using namespace process::null_model;

NullModel::NullModel() {}

NullModel::~NullModel() {}

void NullModel::init() {}

void NullModel::run() {}

double NullModel::GetStepLength() { return 0; }
