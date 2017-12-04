function myFunction(button) {
    var butn = button;
	var elem = document.getElementById(butn.id+"_pre");
	if(butn.textContent == "+"){
	elem.className = "showElem";
	butn.textContent = "-";
	}
	else{
	elem.className = "noShow";
	butn.textContent = "+";
	}
}