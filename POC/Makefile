MAKEFLAGS				:=	--no-print-directory
SHELL					:=	/bin/sh
INCLUDE-DIR				:=	./include/
SRCS-DIR				:=	srcs/
BUILD-DIR				:=	./.build/
NAME					:=	test_elf
ELF_PARSER-SRC				:=	get_eh_1 \
					      get_eh_2 \
					      get_eh \
					      get_elfsh_data_1 \
					      get_elfsh_data_2 \
					      get_elfsh_data_3 \
					      get_elfsh_data \
					      get_real_data \
					      validate_elf_h_1 \
					      validate_elf_h \
					      valid_elf_h_attrib \
					      valid_elf_section_header_2 \
					      valid_program_header_2 \
					      valid_program_header \
					      valid_section_header \
					      get_elfh_e_ident \
					      valid_elfh \
					      valid_elf_h_2 \

ELF_PARSER-DIR                         :=      elf/parser/

ELF_PARSER-SRCS			:=	$(addprefix $(ELF_PARSER-DIR), $(ELF_PARSER-SRC))

CMN-SRC				:= 	overflow \

CMN-DIR                                :=      cmn/

CMN-SRCS				:=	$(addprefix $(CMN-DIR), $(CMN-SRC))

PROGRAM-SRC				:=	file_validation\
					      inject_code\
					      signature_check\

#PROGRAM-DIR                            := ./

#PROGRAM-SRCS				:=	$(addprefix $(PROGRAM-DIR), $(PROGRAM-SRC))
CHECK_PACKER			:= test_check_packer
POC_CHECK_PACKER			:= elf/POC/check_packer

SRC					:= $(PROGRAM-SRC) $(CMN-SRCS) $(ELF_PARSER-SRCS) $(POC_CHECK_PACKER)



BUILD					:=	$(addsuffix .o, $(addprefix $(BUILD-DIR), $(SRC)))

BUILD-POC_CHECK_PACKER		:=	$(addsuffix .o, $(addprefix $(BUILD-DIR), $(POC_CHECK_PACKER)))
# Makefile

# Detect the operating system
UNAME_S := $(shell uname -s)

# Set the CC variable based on the operating system
ifeq ($(UNAME_S),Linux)
    # Check for Guix
    ifeq ($(shell grep -q 'guix' /etc/os-release && echo yes),yes)
        CC := gcc
    else
        CC := cc
    endif
else
    $(error Unsupported operating system: $(UNAME_S))
endif
CFLAGS					:= -Wall -Werror -Wextra -g3# -fsanitize=address
INCLUDEFLAGS			:= -I $(INCLUDE-DIR) -I lib/include/
LINKINGFLAGS			:= #-L lib -l ft
RM						:=	rm -rf
LIB-DIR					:= lib
LIB						:= $(LIB-DIR)/libft.a

# For create directory and print
DIR_DUP					=	mkdir -p $(@D)
# RULES

all: $(NAME)

#$(LIB): $(LIB-DIR)
#	$(MAKE) -C $<

$(NAME): $(BUILD)
		$(CC) $(CFLAGS) -o $@ $(BUILD) $(LINKINGFLAGS)

$(CHECK_PACKER): $(BUILD-POC_CHECK_PACKER)
	$(CC) $(CFLAGS) -o $@ $(BUILD-POC_CHECK_PACKER) $(LINKINGFLAGS)

$(BUILD-DIR):
	@mkdir -p $@

$(BUILD-DIR)%.o:        $(SRCS-DIR)%.c
	@$(DIR_DUP)
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) -c $< -o $@

clean:
	@$(RM) $(BUILD-DIR)
#	$(MAKE) $@ -C $(LIB-DIR)

fclean: clean
	@$(RM) $(NAME)
#	$(MAKE) $@ -C $(LIB-DIR)

re: fclean all

info-%:
	@$(MAKE) --dry-run --always-make $* | grep -v "info"

print-%:
	@$(info '$*'='$($*)')

.PHONY: $(NAME) all clean fclean re info-% print-%
