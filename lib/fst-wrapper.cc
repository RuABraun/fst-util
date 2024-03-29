#include "fst/script/fstscript.h"
#include <ctime>
#include "fst/script/arcsort.h"
#include "fst-wrapper.h"
#include <pybind11/pybind11.h>
#include <fstream>
#include <limits>
#include <set>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/include/pybind11/pytypes.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;


double Plus(double a, double b) {
  return -log(exp(-a) + exp(-b));
}


int WrappedFst::AddState() {
  return fst_->AddState();
}

void WrappedFst::SetStart(int state) {
  fst_->SetStart(state);
}

void WrappedFst::SetFinal(int state, double weight) {
  fst::TropicalWeight w(weight);
  fst_->SetFinal(state, fst::script::WeightClass(w));
}

void WrappedFst::Read(std::string fst_fpath) {
  fst_ = fst::script::VectorFstClass::Read(fst_fpath);
}

void WrappedFst::Write(std::string fst_fpath) {
  fst_->Write(fst_fpath);
}

void WrappedFst::WriteArkEntry(std::string key, std::string fst_fpath) {
  std::fstream fs;
  fs.open(fst_fpath, std::fstream::app | std::fstream::binary);
  fs << key;
  fs << " ";
  fst_->Write(fs, fst_fpath);
  fs.close();
}

void WrappedFst::AddArc(int start_state, int next_state, int ilabel, int olabel, double weight) {
  fst::TropicalWeight w(weight);
  fst::StdArc arc(ilabel, olabel, w, next_state);
  fst::script::ArcClass arcc(arc);
  fst_->AddArc(start_state, arcc);
}

int WrappedFst::AddArcState(int start_state, int ilabel, int olabel, double weight) {
  int next_state = this->AddState();
  fst::TropicalWeight w(weight);
  fst::StdArc arc(ilabel, olabel, w, next_state);
  fst::script::ArcClass arcc(arc);
  fst_->AddArc(start_state, arcc);
  return next_state;
}

int WrappedFst::GetStart() const {
  return fst_->Start();
}

double WrappedFst::Final(int state) const {
  return fst_->Final(state).GetWeight<fst::TropicalWeight>()->Value();
}

std::vector<Arc> WrappedFst::GetArcs(int state) const {
  fst::script::MutableArcIteratorClass arc_iterator(fst_, state);
  std::vector<Arc> vec;
  while (!arc_iterator.Done()) {
    fst::script::ArcClass arcc = arc_iterator.Value();
    Arc arc(arcc.ilabel, arcc.olabel, arcc.weight.GetWeight<fst::TropicalWeight>()->Value(), arcc.nextstate);
    vec.push_back(arc);
    arc_iterator.Next();
  }
  return vec;
}

void WrappedFst::Determinize() {
  const auto weight_threshold = fst::script::WeightClass::Zero(fst_->WeightType());
  fst::script::DeterminizeOptions opts(0.000976562, weight_threshold);
  fst::script::VectorFstClass* new_fst = new fst::script::VectorFstClass(fst_->ArcType());
  fst::script::Determinize(*fst_, new_fst, opts);
  delete fst_;
  fst_ = new_fst;
}

void WrappedFst::Minimize() {
  fst::script::Minimize(fst_, nullptr, 0.000976562, false);
}

void WrappedFst::ArcSort(std::string s) {
  if (s == "ilabel") {
    fst::script::ArcSort(fst_, fst::script::ILABEL_SORT);
  } else {
    fst::script::ArcSort(fst_, fst::script::OLABEL_SORT);
  }
}

void WrappedFst::Compose(WrappedFst &other_fst) {
  fst::script::VectorFstClass* new_fst = new fst::script::VectorFstClass(fst_->ArcType());
  fst::script::Compose(*fst_, *(other_fst.fst_), new_fst);
  delete fst_;
  fst_ = new_fst;
}

void WrappedFst::ShortestPath() {
  fst::script::VectorFstClass* new_fst = new fst::script::VectorFstClass(fst_->ArcType());
  const auto weight_threshold = fst::script::WeightClass::Zero(fst_->WeightType());
  fst::QueueType queue_type;
  fst::script::GetQueueType("auto", &queue_type);
  const fst::script::ShortestPathOptions opts(queue_type, 1, true, 0.000976562, weight_threshold);
  fst::script::ShortestPath(*fst_, new_fst, opts);
  delete fst_;
  fst_ = new_fst;
}

std::vector<int> WrappedFst::States() const {
  fst::script::StateIteratorClass state_iterator(*fst_);
  std::vector<int> vec;
  while (!state_iterator.Done()) {
    int state = state_iterator.Value();
    vec.push_back(state);
    state_iterator.Next();
  }
  return vec;
}

void WrappedFst::Connect() {
  fst::script::Connect(fst_);
}

void WrappedFst::DeleteArcs(int state) {
  fst_->DeleteArcs(state);
}

int WrappedFst::NumStates() const {
  return fst_->NumStates();
}

int WrappedFst::NumArcs(int state) const {
  return fst_->NumArcs(state);
}

void WrappedFst::ReplaceSimple(const int olabel, std::vector<int> wordlist) {
  for (int state: this->States()) {
    std::vector<Arc> arcs = this->GetArcs(state);
    std::vector<Arc> arcs_to_keep;
    Arc arc_to_replace;
    arcs_to_keep.reserve(arcs.size());
    bool delete_arcs = false;
    for (Arc arc: arcs) {
      if (arc.olabel == olabel) {
        delete_arcs = true;
        arc_to_replace = arc;
      } else {
        arcs_to_keep.push_back(arc);
      }
    }

    if (delete_arcs) {
      this->DeleteArcs(state);
      for (Arc arc: arcs_to_keep) {
        this->AddArc(state, arc.nextstate, arc.ilabel, arc.olabel, arc.weight);
      }

      for (int word: wordlist) {
        this->AddArc(state, arc_to_replace.nextstate, word, olabel, arc_to_replace.weight);
      }
    }
  }
}


void WrappedFst::Insert(const int olabel, WrappedFst* fst) {
  for (int state: this->States()) {
    std::vector<Arc> arcs = this->GetArcs(state);
    std::vector<Arc> arcs_to_keep;
    Arc arc_to_replace;
    arcs_to_keep.reserve(arcs.size());
    bool delete_arcs = false;
    for (Arc arc: arcs) {
      if (arc.olabel == olabel) {
        delete_arcs = true;
        arc_to_replace = arc;
      } else {
        arcs_to_keep.push_back(arc);
      }
    }

    if (delete_arcs) {
      this->DeleteArcs(state);
      for (Arc arc: arcs_to_keep) {
        this->AddArc(state, arc.nextstate, arc.ilabel, arc.olabel, arc.weight);
      }


      int start_state = -1;
      int offset = this->NumStates();
      std::vector<int> finals;
      int sub_start_state = fst->GetStart();
      for (int substate: fst->States()) {
        int n = this->AddState();
        if (substate == sub_start_state) start_state = n;
        if (fst->Final(substate) != std::numeric_limits<double>::infinity()) {
          finals.push_back(substate);
        }
      }
      if (start_state == -1) throw "WTF!";
      this->AddArc(state, start_state, 0, 0, arc_to_replace.weight + 2.3);

      for (int substate: fst->States()) {
        for (Arc arc: fst->GetArcs(substate)) {
          this->AddArc(substate + offset, arc.nextstate + offset, arc.ilabel, arc.olabel, 0.);
        }
      }

      for (int final: finals) {
        this->AddArc(final + offset, arc_to_replace.nextstate, 0, 0, 0);
      }

    }
  }
}

void WrappedFst::ReplaceSingle(const int olabel, WrappedFst* fst, double weight) {
  // If weight is zero will use unk weight
  // Assumes arcs with olabel all go to the same state
  int destination_state = -1;
  std::vector<std::pair<int, Arc>> arcs_to_replace;
  for (int state: this->States()) {
    std::vector<Arc> arcs = this->GetArcs(state);
    std::vector<Arc> arcs_to_keep;
    Arc arc_to_replace;
    arcs_to_keep.reserve(arcs.size());
    bool delete_arcs = false;
    for (Arc arc: arcs) {
      if (arc.olabel == olabel) {
        delete_arcs = true;
        arc_to_replace = arc;
      } else {
        arcs_to_keep.push_back(arc);
      }
    }

    if (delete_arcs) {
      DeleteArcs(state);
      for (Arc arc: arcs_to_keep) {
        AddArc(state, arc.nextstate, arc.ilabel, arc.olabel, arc.weight);
      }
      if (destination_state == -1) {
        destination_state = arc_to_replace.nextstate;
      } else if (destination_state != arc_to_replace.nextstate) {
        std::cerr << "Assumption broken! " << destination_state<< " "<<arc_to_replace.nextstate<<std::endl;
      }
      if (state != arc_to_replace.nextstate) {
        arcs_to_replace.emplace_back(state, arc_to_replace);
      }

    }
  }

  int offset = this->NumStates();
  int start_state = -1;
  std::vector<int> finals;
  int sub_start_state = fst->GetStart();
  for (int substate: fst->States()) {
    int n = this->AddState();
    if (substate == sub_start_state) start_state = n;
    if (fst->isFinal(substate)) {
      finals.push_back(substate);
    }
  }
  if (start_state == -1) throw "WTF!";

  for (std::pair<int, Arc> pair: arcs_to_replace) {
    int state = pair.first;
    Arc arc = pair.second;
    double new_weight = weight == 0. ? arc.weight + 2.3 : weight;
    this->AddArc(state, start_state, 0, 0, new_weight);
  }
  for (int substate: fst->States()) {
    for (Arc arc: fst->GetArcs(substate)) {
      this->AddArc(substate + offset, arc.nextstate + offset, arc.ilabel, arc.olabel, 0.);
    }
  }

  for (int final: finals) {
    this->AddArc(final + offset, destination_state, 0, 0, 0);
  }
}


void WrappedFst::ReplaceSingleLooped(const int olabel, WrappedFst* fst) {
  // Assumes arcs with olabel all go to the same state, works on FST with self loops.
  int destination_state = -1;
  std::vector<std::pair<int, Arc>> arcs_to_replace;
  for (int state: this->States()) {
    std::vector<Arc> arcs = this->GetArcs(state);
    std::vector<Arc> arcs_to_keep;
    Arc arc_to_replace;
    arcs_to_keep.reserve(arcs.size());
    bool delete_arcs = false;
    for (Arc arc: arcs) {
      if (arc.olabel == olabel) {
        delete_arcs = true;
        arc_to_replace = arc;
      } else {
        arcs_to_keep.push_back(arc);
      }
    }

    if (delete_arcs) {
      DeleteArcs(state);
      for (Arc arc: arcs_to_keep) {
        AddArc(state, arc.nextstate, arc.ilabel, arc.olabel, arc.weight);
      }
      if (destination_state == -1) {
        destination_state = arc_to_replace.nextstate;
      } else if (destination_state != arc_to_replace.nextstate) {
        std::cerr << "Assumption broken! " << destination_state<< " "<<arc_to_replace.nextstate<<std::endl;
      }
      if (state != arc_to_replace.nextstate) {
        arcs_to_replace.emplace_back(state, arc_to_replace);
      }

    }
  }

  int offset = this->NumStates();
  int start_state = -1;
  std::vector<int> finals;
  int sub_start_state = fst->GetStart();
  for (int substate: fst->States()) {
    int n = this->AddState();
    if (substate == sub_start_state) start_state = n;
    if (fst->isFinal(substate)) {
      finals.push_back(substate);
    }
  }
  if (start_state == -1) throw "WTF!";

  for (std::pair<int, Arc> pair: arcs_to_replace) {
    int state = pair.first;
    Arc arc = pair.second;
    this->AddArc(state, start_state, 0, 0, arc.weight + 2.3);
  }

  // To keep track of which states we've added self loops to
  std::unordered_set<int> sl_added;
  for (int substate: fst->States()) {
    for (Arc arc: fst->GetArcs(substate)) {
      this->AddArc(substate + offset, arc.nextstate + offset, arc.ilabel, arc.olabel, 0.69);
      if (arc.ilabel != 0 && sl_added.find(arc.nextstate+offset) == sl_added.end()) {
        this->AddArc(arc.nextstate + offset, arc.nextstate + offset, arc.ilabel, 0, 0.69);
        sl_added.insert(arc.nextstate + offset);
      }
    }
  }

  for (int final: finals) {
    this->AddArc(final + offset, destination_state, 0, 0, 0);
  }
}

//bool WrappedFst::CheckHasEpsilonLoop(int start, int end) {
//  int current_state = start;
//  while (true) {
//    ArcIterator arc_iterator(*this, current_state);
//    while (!arc_iterator.Done()) {
//      Arc arc = arc_iterator.Value();
//      arc_iterator.Next();
//    }
//  }
//}

WrappedFst* WrappedFst::Copy() const {
  WrappedFst* f = new WrappedFst;
  for (int state: this->States()) {
    f->AddState();
    if (this->isFinal(state)) {
      f->SetFinal(state);
    }
  }
  for (int state: this->States()) {
    for (Arc& arc: this->GetArcs(state)) {
      f->AddArc(state, arc.nextstate, arc.ilabel, arc.olabel, arc.weight);
    }
  }
  f->SetStart(this->GetStart());
  return f;
}


std::vector< std::vector<int>> WrappedFst::RandPath(int n) const {
  srand((unsigned)time(0));
  std::vector< std::vector<int>> lst(n);
  for (int k = 0; k < n; ++k) {
      int state = this->GetStart();
      std::vector<int> labels;
      double num = this->Final(state);
      while (!this->isFinal(state)) {
        std::vector<Arc> arcs = this->GetArcs(state);
        int i = rand() % arcs.size();
        Arc arc = arcs[i];
        labels.push_back(arc.olabel);
        state = arc.nextstate;
      }
      lst[k] = labels;
  }
  return lst;
}


void WrappedFst::NormaliseWeights() {
  for (int state: this->States()) {
    double sum = this->Final(state);
    if (sum == std::numeric_limits<double>::infinity()) sum = 0.;
    for (Arc arc: this->GetArcs(state)) {
      sum += exp(-arc.weight);
    }
    ArcIterator arc_iterator(*this, state);
    while (!arc_iterator.Done()) {
      Arc arc = arc_iterator.Value();
      arc_iterator.SetValue(Arc(arc.ilabel, arc.olabel, -log( exp(-arc.weight) / sum), arc.nextstate));
      arc_iterator.Next();
    }
  }
}


int pairing_function(int a, int b) {
  return (a + b) * (a + b + 1) / 2 + b;
}

void WrappedFst::AddBoost(std::vector< std::vector<int>> word_subwords, double boost, int disambig, int unk) {
  int noctx_start = -1;
  for (Arc arc: GetArcs(GetStart())) {
    if (arc.ilabel == disambig) {
      noctx_start = arc.nextstate;
      break;
    }
  }

  // Getting unigram states
  std::unordered_map<int, int> subword_to_state;
  for (Arc arc: GetArcs(noctx_start)) {
    if (arc.ilabel == disambig) continue;
    subword_to_state[arc.ilabel] = arc.nextstate;
  }

  std::unordered_set<int> states_adjusted;
  for (std::vector<int>& subwords: word_subwords) {
//    if (subwords.size() > 5) {
//      std::cerr << "Too big!!"<<std::endl;
//      throw "WTF";
//    }

    if (noctx_start == -1) throw "WTF";
    // First going to create if necessary and boost probability of arcs with subwords from noctx_start
    {
      bool found;
      int current_state = noctx_start;
      std::vector<std::pair<int, int>> state_arc_s;
      int subword_idx = 0;
      while (true) {
        ArcIterator arc_iterator(*this, current_state);
        found = false;
        while (!arc_iterator.Done()) {
          Arc arc = arc_iterator.Value();
          if (arc.ilabel == subwords[subword_idx]) {
            state_arc_s.emplace_back(current_state, arc_iterator.count_next_);  // actually current state
            current_state = arc.nextstate;
            found = true;
            break;
          }
          arc_iterator.Next();
        }
        if (found) {
          ++subword_idx;
          if (subword_idx == subwords.size()) {  // found all
            break;
          }
        } else {
          break;
        }
      }
      // Adjusting arks of states found
      for (std::pair<int, int> state_arc: state_arc_s) {
        int state = state_arc.first, arc_i = state_arc.second;
        ArcIterator arc_iter(*this, state);
        arc_iter.NextI(arc_i);
        Arc arc = arc_iter.Value();

        int pair_val = pairing_function(state, arc.ilabel);
        if (states_adjusted.find(pair_val) == states_adjusted.end()) {
          arc_iter.SetValue(Arc(arc.ilabel, arc.olabel, arc.weight - log(boost), arc.nextstate));
          states_adjusted.insert(pair_val);
        }
      }

      if (subword_idx != subwords.size()) {
        // Adding arcs if not complete sequence found
        std::pair<int, int> state_arc = state_arc_s.back();
        int state = state_arc.first, arc_i = state_arc.second;
        ArcIterator arc_iter_tmp(*this, state);
        arc_iter_tmp.NextI(arc_i);
        Arc arc = arc_iter_tmp.Value();
        state = arc.nextstate;

        int new_state;
        while (subword_idx < subwords.size()) {
          double weight = 0.;
          if (state == arc.nextstate) {  // first added arc
            if (subword_idx == 1) {
              weight = 2.3;
            } else {
              weight = 0.69;
            }
          }
          new_state = AddState();
//      std::cout << state<< " "<<new_state<<" "<<subwords[subword_idx]<<std::endl;
          AddArc(state, new_state, subwords[subword_idx], subwords[subword_idx], weight);
          state = new_state;
          ++subword_idx;
        }
//        AddArc(new_state, noctx_start, disambig, 0, 0.);

        // Adding arcs going to unigram state
        AddArc(new_state, subword_to_state[subwords.back()], disambig, 0, 0.);
      }
    }

    // Creating arcs from start state (<s>)

    // Now boosting arcs to unigram state of first subword
//  std::pair<int, int> state_arc = state_arc_s[0];
//  int state = state_arc.first, arc_i = state_arc.second;
//  ArcIterator arc_iter_tmp(*this, state);
//  arc_iter_tmp.NextI(arc_i);
//  Arc arc = arc_iter_tmp.Value();
//
//  int target_state = arc.nextstate;
//  for (int state: States()) {
//    ArcIterator arc_iterator(*this, state);
//    while (!arc_iterator.Done()) {
//      Arc arc = arc_iterator.Value();
//      if (arc.nextstate == target_state) {
//        if (arc.ilabel != subwords[0] && arc.ilabel != disambig) {
//          std::cout << arc.ilabel<<" "<<subwords[0]<<std::endl;
//          throw "WTF labels";
//        }
//        arc_iterator.SetValue(Arc(arc.ilabel, arc.olabel, arc.weight - log(boost), target_state));
//      }
//      arc_iterator.Next();
//    }
//  }
    ArcSort("ilabel");
  }
}


PYBIND11_MODULE(wrappedfst, m) {
  m.doc() = "pybind11 plugin";

  py::class_<Arc>(m, "Arc")
    .def(py::init<int, int, double, int>())
    .def_readwrite("ilabel", &Arc::ilabel)
    .def_readwrite("olabel", &Arc::olabel)
    .def_readwrite("weight", &Arc::weight)
    .def_readwrite("nextstate", &Arc::nextstate);

  py::class_<WrappedFst>(m, "WrappedFst")
    .def(py::init<>())
    .def(py::init<std::string>())
    .def(py::init<const WrappedFst>())
    .def("read", &WrappedFst::Read)
    .def("write", &WrappedFst::Write)
    .def("write_ark_entry", &WrappedFst::WriteArkEntry)
    .def("read_ark_entries", &WrappedFst::ReadArkEntries)
    .def("add_state", &WrappedFst::AddState)
    .def("set_start", &WrappedFst::SetStart)
    .def("set_final", &WrappedFst::SetFinal, py::arg("state"),py::arg("weight")=0.)
    .def("add_arc", &WrappedFst::AddArc)
    .def("add_arcstate", &WrappedFst::AddArcState)
    .def("get_start", &WrappedFst::GetStart)
    .def("get_arcs", &WrappedFst::GetArcs)
    .def("determinize", &WrappedFst::Determinize)
    .def("minimize", &WrappedFst::Minimize)
    .def("arc_sort", &WrappedFst::ArcSort)
    .def("compose", &WrappedFst::Compose)
    .def("shortest_path", &WrappedFst::ShortestPath)
    .def("final", &WrappedFst::Final)
    .def("is_final", &WrappedFst::isFinal)
    .def("states", &WrappedFst::States)
    .def("connect", &WrappedFst::Connect)
    .def("delete_arcs", &WrappedFst::DeleteArcs)
    .def("delete_states", &WrappedFst::DeleteStates)
    .def("num_states", &WrappedFst::NumStates)
    .def("num_arcs", &WrappedFst::NumArcs)
    .def("replace_simple", &WrappedFst::ReplaceSimple)
    .def("insert", &WrappedFst::Insert)
    .def("replace_single", &WrappedFst::ReplaceSingle, py::arg("olabel"), py::arg("fst"), py::arg("weight")=0.)
    .def("replace_single_looped", &WrappedFst::ReplaceSingleLooped)
    .def("add_boost", &WrappedFst::AddBoost)
    .def("normalise_weights", &WrappedFst::NormaliseWeights)
    .def("copy", &WrappedFst::Copy,  py::return_value_policy::take_ownership)
    .def("randpath", &WrappedFst::RandPath)
    .def(py::pickle(
      [](const WrappedFst& f) {
        int num_states = f.NumStates();
        int start_state = f.GetStart();
        py::list final_states;
        py::list arcs;
        for (int state: f.States()) {
          if (f.Final(state) == 0.) {
            final_states.append(py::int_(state));
          }
          for (Arc& arc: f.GetArcs(state)) {
            arcs.append(py::make_tuple(py::int_(state), py::int_(arc.nextstate), py::int_(arc.ilabel), py::int_(arc.olabel), py::float_(arc.weight)));
          }
        }
        return py::make_tuple(num_states, start_state, final_states, arcs);
      },
      [](py::tuple t) {
        WrappedFst f;
        int num_states = t[0].cast<int>();
        int start_state = t[1].cast<int>();
        py::list final_states = t[2];
        py::list arcs = t[3];
        for (int i = 0; i < num_states; i++) {
          int state = f.AddState();
          if (state == start_state) f.SetStart(state);
        }
        for (py::handle obj: final_states) {
          int state = obj.cast<int>();
          f.SetFinal(state);
        }
        for (py::handle obj: arcs) {
          py::tuple tpl = obj.cast<py::tuple>();
          f.AddArc(tpl[0].cast<int>(), tpl[1].cast<int>(), tpl[2].cast<int>(), tpl[3].cast<int>(), tpl[4].cast<double>());
        }
        return f;
      }
      ))
      .def("__copy__", [](const WrappedFst& wfst) {
        return WrappedFst(wfst);
      })
      .def("__deepcopy__", [](const WrappedFst& wfst) {
        return WrappedFst(wfst);
      });

  py::class_<ArcIterator>(m, "ArcIterator")
    .def(py::init<WrappedFst&, int>())
    .def("Done", &ArcIterator::Done)
    .def("Next", &ArcIterator::Next)
    .def("Value", &ArcIterator::Value)
    .def("SetValue", &ArcIterator::SetValue);

}
