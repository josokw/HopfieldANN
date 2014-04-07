CXX = gcc 
CXXFLAGS = -Wall -Wextra -O3

EXECUTABLE = hopfield
SOURCES = ${wildcard *.cpp}
HEADERS = ${wildcard *.h}
OBJECTS = ${SOURCES:.cpp=.o}

.PHONY: all
all: ${EXECUTABLE}

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LDXXFLAGS) -o $@
	@echo "-- Build: " $$(cat buildnumber.num)

# Create dependency file
depend: $(SOURCES)
	$(CXX) $(CXXFLAGS) -MM  $^ > $@

-include depend

# Create a clean environment
.PHONY: clean
clean:
	$(RM) $(EXECUTABLE) $(OBJECTS)

# Clean up dependency file  
.PHONY: clean-depend
clean-depend: clean
	$(RM) depend  
	                                              
