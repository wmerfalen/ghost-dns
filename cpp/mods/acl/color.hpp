#define grn_text(a){ std::cout << ::mods::Color::fg::green << a << "\n"; rst_text(); }
#define red_text(a){ std::cout << ::mods::Color::fg::red << a << "\n"; rst_text(); }
#define blu_text(a){ std::cout << ::mods::Color::fg::blue << a << "\n"; rst_text(); }
#define rst_text(){ std::cout << ::mods::Color::fg::def; }
namespace mods {
	namespace Color {
		enum Code {
			FG_RED      = 31,
			FG_GREEN    = 32,
			FG_BLUE     = 34,
			FG_DEFAULT  = 39,
			BG_RED      = 41,
			BG_GREEN    = 42,
			BG_BLUE     = 44,
			BG_DEFAULT  = 49
		};
		class Modifier {
			Code code;
		public:
			Modifier(Code pCode) : code(pCode) {}
			friend std::ostream&
			operator<<(std::ostream& os, const Modifier& mod) {
				return os << "\033[" << mod.code << "m";
			}
		};
        namespace fg {
            static const Modifier red(Color::FG_RED);    
            static const Modifier green(Color::FG_GREEN);    
            static const Modifier blue(Color::FG_BLUE);    
            static const Modifier def(Color::FG_DEFAULT);    
        };
        namespace bg {
            static const Modifier red(Color::BG_RED);    
            static const Modifier green(Color::BG_GREEN);    
            static const Modifier blue(Color::BG_BLUE);    
            static const Modifier def(Color::BG_DEFAULT);    
        };
	};
    namespace colors {
        const std::string red("\033[0;31m");
        const std::string reset("\033[0m");
    };

};
