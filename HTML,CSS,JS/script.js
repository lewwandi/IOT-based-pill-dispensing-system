var firebaseConfig = {
    apiKey: "AIzaSyDi3DIZm9jGJYTcDGtK3cIey46Hk0hW_KY",
    authDomain: "pill-dispensing-system-6c2b6.firebaseapp.com",
    databaseURL: "https://pill-dispensing-system-6c2b6-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "pill-dispensing-system-6c2b6",
    storageBucket: "pill-dispensing-system-6c2b6.appspot.com",
    messagingSenderId: "736108324154",
    appId: "1:736108324154:web:edb0515248a079f5189c03",
    measurementId: "G-GXKF8BSPT3"
};

// Initialize Firebase
firebase.initializeApp(firebaseConfig);

// Get a reference to the database service
const database = firebase.database();

function saveTimes() {
    console.log('saveTimes function called');

    const time1 = document.getElementById('time1').value.split(':');
    const time2 = document.getElementById('time2').value.split(':');
    const time3 = document.getElementById('time3').value.split(':');

    // Ensure values are present before trying to save
    if (!time1.length || !time2.length || !time3.length) {
        console.error('Time inputs are incomplete.');
        return;
    }

    const timesRef = database.ref('times');
    timesRef.set({
        time1: { hour: parseInt(time1[0], 10), minute: parseInt(time1[1], 10) },
        time2: { hour: parseInt(time2[0], 10), minute: parseInt(time2[1], 10) },
        time3: { hour: parseInt(time3[0], 10), minute: parseInt(time3[1], 10) },
        timestamp: firebase.database.ServerValue.TIMESTAMP
    })
        .then(() => {
            console.log('Times saved successfully!');
            // Optional: Update the UI to show that the times were saved successfully
        })
        .catch((error) => {
            console.error('Error saving times: ', error);
            // Optional: Update the UI to indicate an error
        });
}

// Attach event listener to the saveButton when the DOM is fully loaded
document.addEventListener('DOMContentLoaded', function () {
    var saveButton = document.getElementById('saveButton');
    saveButton.addEventListener('click', saveTimes);
});