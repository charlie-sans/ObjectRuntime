"""Configuration helpers for the ObjectIR Python runtime."""

PRELOADED_STD_LIB_MODULES = [
	# Modules listed here are imported when the runtime initializes so their
	# classes and functions are available for ObjectIR method calls.
	"Generics",
]
