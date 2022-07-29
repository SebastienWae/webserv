#include "Forms.hpp"

Forms::~Forms() {}

std::string Forms::Sendfile(void) {
  std::string str;
  str
      = "<form method=\"post\"  enctype=\"multipart/form-data\">   <div>     <label "
        "for=\"file\">Choisir un fichier</label>     <input type=\"file\" id=\"file\" name=\"myFile\">   </div>   "
        "<div>     <button>Envoyer le fichier</button>   </div> </form>";

  return (str);
}

std::string Forms::Sendform(void) {
  std::string str;
  str
      = " <form method=\"POST\">   <div>     <label for=\"say\">Quelle salutation "
        "voulez-vous dire ?</label>     <input name=\"say\" id=\"say\" value=\"Salut\">   </div>   <div>     <label "
        "for=\"to\">A qui voulez-vous le dire ?</label>     <input name=\"to\" id=\"to\" value=\"Maman\">   </div>   "
        "<div>     <button>Envoyer mes salutations</button>   </div> </form>  ";

  return (str);
}