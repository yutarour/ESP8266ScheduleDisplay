const char schedule_page[] PROGMEM = R"=====(
  <html>
    <body>
        <h1 style="text-align: center;">Scheduler</h1>
        <div>
            <h3>Add Event</h3>
            <form action="/add" method="get">
                <input type="number" name="year" id="year" placeholder="Year"/>
                <br>
                <input type="number" name="month" id="month" placeholder="Month"/>
                <br>
                <input type="number" name="day" id="day" placeholder="Day"/>
                <br>
                <input type="number" name="hour" id="hour" placeholder="Hour"/>
                <br>
                <input type="number" name="minute" id="minute" placeholder="Minute"/>
                <br>
                <input type="number" name="second" id="second" placeholder="Second"/>
                <br>
                <input type="text" name="event" id="name" placeholder="Event"/>
                <br>
                <input type="submit" value="Submit"/>
            </form>
        </div>
        <br>
        <div>
            <h3>Toggle Backlight</h3>
            <form action="/backlight" method="get">
                <input type="submit" value="Toggle Backlight"/>
            </form>
        </div>
        <br>
        <div>
            <h3>Remove Event</h3>
            <form action="/removeEvent" method="get">
                <input type="text",name="eventname",id="eventname">
                <input type="submit" value="Submit">
            </form>
        </div>
    </body>
</html>
)=====";